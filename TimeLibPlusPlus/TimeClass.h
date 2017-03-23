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

#ifndef _TIMECLASS_
#define _TIMECLASS_ 1

#include <ESP8266WiFi.h>
#include "defines.h"

typedef enum {timeNotSet, timeNeedsSync, timeSet
}  timeStatus_t ;

typedef enum {
	dowInvalid, dowSunday, dowMonday, dowTuesday, 
	dowWednesday, dowThursday, dowFriday, dowSaturday
} timeDayOfWeek_t;

typedef enum {
	tmSecond, tmMinute, tmHour, tmWday, 
	tmDay, tmMonth, tmYear, tmNbrFields
} tmByteFields;	   

typedef struct  { 
	uint8_t Second; 
	uint8_t Minute; 
	uint8_t Hour; 
	uint8_t Wday;   // day of week, sunday is day 1
	uint8_t Day;
	uint8_t Month; 
	uint8_t Year;   // offset from 1970; Seems more like 2000.
} 	tmElements_t;


class TimeClass{
protected:
	TimeClass();
	void refreshCache(time_t t);	
	virtual time_t getTimeFromSource()=0;
	/* low level functions to convert to and from system time                     */
	void breakTime(time_t time, tmElements_t &tm);  // break time_t into elements
	time_t makeTime(tmElements_t &tm, uint16_t _offset);  // convert time elements into time_t
private:
	//int8_t timeZone;
	uint32_t sysTime = 0;
	uint32_t prevMillis = 0;
	uint32_t nextSyncTime = 0;
	timeStatus_t Status = timeNotSet;
	char buffer[dt_MAX_STRING_LEN+1];  // must be big enough for longest string and the terminating null

	const uint8_t daysInAMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; //API starts months from 1, this array starts from 0
	
	tmElements_t tm;    // a cache of time elements
	time_t cacheTime;   // the time the cache was updated
	uint32_t syncInterval = 300;  // default time sync will be attempted after this many seconds

public:
	
	bool 	isAM();            // returns true if time now is AM
	bool 	isPM();            // returns true if time now is PM
	int     hourFormat12(); // the hour for the given time in 12 hour format

	time_t now();              // return the current time as seconds since Jan 1 1970 
	void    setTime(time_t t);
	//void    setTime(int hr,int min,int sec,int day, int month, int yr);
	void    adjustTime(long adjustment);

	/* date strings */ 
	char* monthStr(uint8_t month);
	char* monthShortStr(uint8_t month);
	char* dayStr(uint8_t day);
	char* dayShortStr(uint8_t day);

	timeStatus_t timeStatus(); // indicates if time has been set and recently synchronized
	void init(time_t _syncInterval);//, int8_t _timeZone);
	tmElements_t& getTimeElements();  
	
};

#endif //_TIMECLASS_
