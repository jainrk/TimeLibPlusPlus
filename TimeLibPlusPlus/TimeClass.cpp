/*	

	Coded by: Rajendra Jain on March 23 2017.
	This is my attempt to create a library of classes dealing with time.
	The background work comes from the TimeLib
	The Virtual base class TimeClass maintains the methods to maintain manage 
	time within a local clock. It updates the local clock from a time source every 
	syncInterval via the virtual function virtual time_t getTimeFromSource()=0;
	The derived classes DS1307RTCClass, and NISTClass provide the time every 
	syncInterval through calls made via getTimeFromSource().
	
	This project is licensed under the terms of the MIT license.

*/

#include <string.h> // for strcpy_P or strcpy
#include "TimeClass.h"
#include "NISTClass.h"
#include <WiFiUdp.h>

TimeClass::TimeClass(){
}

void TimeClass::refreshCache(time_t t) {
	if (t != cacheTime) {
		breakTime(t, tm); 
		cacheTime = t; 
	}
}

int TimeClass::hourFormat12() { // the hour for the given time in 12 hour format
	refreshCache(now());
	if( tm.Hour == 0 )
	return 12; // 12 midnight
	else if( tm.Hour  > 12)
	return tm.Hour - 12 ;
	else
	return tm.Hour ;
}

bool TimeClass::isAM() { // returns true if time now is AM
	refreshCache(now());
	return (tm.Hour < 12); 
}

bool TimeClass::isPM() { // returns true if PM
	refreshCache(now());
	return (tm.Hour >= 12); 
}

/*============================================================================*/	
/* Methods to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

void TimeClass::breakTime(time_t timeInput, tmElements_t &tm){
	// break the given time_t into time components
	// this is a more compact version of the C library localtime function
	// note that year is offset from 1970 !!!

	uint8_t year;
	uint8_t month, monthLength;
	uint32_t time;
	unsigned long days;

	time = (uint32_t)timeInput;
	tm.Second = time % 60;
	time /= 60; // now it is minutes
	tm.Minute = time % 60;
	time /= 60; // now it is hours
	tm.Hour = time % 24;
	time /= 24; // now it is days
	tm.Wday = ((time+1) % 7) + 1;  // Sunday is day 1 it was + 4

	year = 0;  
	days = 0;
	while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	tm.Year = year-100; // year is offset from 1900 

	days -= LEAP_YEAR(year) ? 366 : 365;
	time  -= days; // now it is days in this year, starting at 0

	days=0;
	month=0;
	monthLength=0;
	for (month=0; month<12; month++) {
		if (month==1) { // february
			if (LEAP_YEAR(year)) {
				monthLength=29;
			} else {
				monthLength=28;
			}
		} else {
			monthLength = daysInAMonth[month];
		}
		
		if (time >= monthLength) {
			time -= monthLength;
		} else {
			break;
		}
	}
	tm.Month = month + 1;  // jan is month 1  
	tm.Day = time + 1;     // day of month
	//tm.Year += 1900;// adjust for years from 1900
}

time_t TimeClass::makeTime(tmElements_t &_tm, uint16_t _offset=0){   
	int i;
	uint32_t seconds;
	// seconds from 1970 till 1 jan 00:00:00 of the given year
	//Serial.println(_tm.Year, DEC);
	uint16_t years = _tm.Year + _offset;
	seconds= years*(SECS_PER_DAY * 365);
	for (i = 0; i < years; i++) {
		if (LEAP_YEAR(i)) {
			seconds +=  SECS_PER_DAY;   // add extra days for leap years
		}
	}

	// add days for this year, months start from 1
	for (i = 1; i < 3; i++) {
		if ( (i == 2) && LEAP_YEAR(_tm.Year)) { 
			seconds += SECS_PER_DAY * 29;
		} else {
			seconds += SECS_PER_DAY * daysInAMonth[i-1];  //monthDay array starts from 0
		}
	}
	seconds+= (_tm.Day-1) * SECS_PER_DAY;
	seconds+= (_tm.Hour)* SECS_PER_HOUR; // - timeZone
	seconds+= _tm.Minute * SECS_PER_MIN;
	seconds+= _tm.Second;
	return (time_t)seconds; 
}

tmElements_t& TimeClass::getTimeElements(){ 
	refreshCache((time_t)sysTime);
	return tm; 
}	

time_t TimeClass::now(){	
	while (millis() - prevMillis >= 1000) {
		sysTime++;
		prevMillis += 1000;	
	}
	
	if (nextSyncTime <= sysTime) {
		time_t t = getTimeFromSource(); 
		Serial.println("Sync"); //Serial.println((time_t)t); //delay(500);
		if (t != 0) {
			//Serial.print(" time: "); Serial.println(t,DEC); 
			setTime(t);
		} else {
			nextSyncTime = sysTime + syncInterval;
			Status = (Status == timeNotSet) ?  timeNotSet : timeNeedsSync;
		}
	}
	
	return (time_t)sysTime; 
}

void TimeClass::init(time_t _syncInterval){//, int8_t _timeZone = 0){
	if(_syncInterval < 60) _syncInterval = 60;
	syncInterval = (uint32_t)_syncInterval;
	//timeZone = _timeZone;
	time_t t = 0;
	Serial.println("Initalizing"); 
	while (t == 0) {
		Serial.print("."); 
		if (t) Serial.println(t,DEC); 
		t = (time_t)getTimeFromSource();//ntpClient->getUNIXTime();		
		setTime(t);
		delay(1000);		
	}
}

void TimeClass::setTime(time_t t) { 
	//t += (time_t)(timeZone) * SECS_PER_HOUR;

	sysTime = (uint32_t)t;  
	nextSyncTime = (uint32_t)t + syncInterval;
	Status = timeSet;
	prevMillis = millis();  // restart counting from now 
} 

void TimeClass::adjustTime(long adjustment) {
	sysTime += adjustment;
}

// indicates if time has been set and recently synchronized
timeStatus_t TimeClass::timeStatus() {
	now(); // required to actually update the status
	return Status;
}


//====================================================================================================

const char monthStr0[] PROGMEM = "";
const char monthStr1[] PROGMEM = "January";
const char monthStr2[] PROGMEM = "February";
const char monthStr3[] PROGMEM = "March";
const char monthStr4[] PROGMEM = "April";
const char monthStr5[] PROGMEM = "May";
const char monthStr6[] PROGMEM = "June";
const char monthStr7[] PROGMEM = "July";
const char monthStr8[] PROGMEM = "August";
const char monthStr9[] PROGMEM = "September";
const char monthStr10[] PROGMEM = "October";
const char monthStr11[] PROGMEM = "November";
const char monthStr12[] PROGMEM = "December";

const PROGMEM char * const PROGMEM monthNames_P[] =
{
	monthStr0,monthStr1,monthStr2,monthStr3,monthStr4,monthStr5,monthStr6,
	monthStr7,monthStr8,monthStr9,monthStr10,monthStr11,monthStr12
};

const char monthShortNames_P[] PROGMEM = "ErrJanFebMarAprMayJunJulAugSepOctNovDec";

const char dayStr0[] PROGMEM = "Err";
const char dayStr1[] PROGMEM = "Sunday";
const char dayStr2[] PROGMEM = "Monday";
const char dayStr3[] PROGMEM = "Tuesday";
const char dayStr4[] PROGMEM = "Wednesday";
const char dayStr5[] PROGMEM = "Thursday";
const char dayStr6[] PROGMEM = "Friday";
const char dayStr7[] PROGMEM = "Saturday";

const PROGMEM char * const PROGMEM dayNames_P[] =
{
	dayStr0,dayStr1,dayStr2,dayStr3,dayStr4,dayStr5,dayStr6,dayStr7
};

const char dayShortNames_P[] PROGMEM = "ErrSunMonTueWedThuFriSat";

/* methods to return date strings */

char* TimeClass::monthStr(uint8_t month){
	strcpy_P(buffer, (PGM_P)pgm_read_word(&(monthNames_P[month])));
	return buffer;
}

char* TimeClass::monthShortStr(uint8_t month){
	for (int i=0; i < dt_SHORT_STR_LEN; i++)      
	buffer[i] = pgm_read_byte(&(monthShortNames_P[i+ (month*dt_SHORT_STR_LEN)]));  
	buffer[dt_SHORT_STR_LEN] = 0;
	return buffer;
}

char* TimeClass::dayStr(uint8_t day) {
	strcpy_P(buffer, (PGM_P)pgm_read_word(&(dayNames_P[day])));
	return buffer;
}

char* TimeClass::dayShortStr(uint8_t day) {
	uint8_t index = day*dt_SHORT_STR_LEN;
	for (int i=0; i < dt_SHORT_STR_LEN; i++)      
	buffer[i] = pgm_read_byte(&(dayShortNames_P[index + i]));  
	buffer[dt_SHORT_STR_LEN] = 0; 
	return buffer;
}

/* 
void TimeClass::setTime(int hr,int min,int sec,int dy, int mnth, int yr){
	// year can be given as full four digit year or two digts (2010 or 10 for 2010);  
	//it is converted to years since 1970
	if( yr > 99)
	yr = yr - 1970;
	else
	yr += 30;  
	tm.Year = yr;
	tm.Month = mnth;
	tm.Day = dy;
	tm.Hour = hr;
	tm.Minute = min;
	tm.Second = sec;
	setTime(makeTime(tm));
}
*/