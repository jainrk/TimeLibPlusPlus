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

I have yet to figure out a good way to adjust for timezone and Daylight Saving Time.

timeZone = 1;     // Central European Time
int timeZone = -5;  // Eastern Standard Time (USA)
timeZone = -4;  // Eastern Daylight Time (USA)
int timeZone = -8;  // Pacific Standard Time (USA)
int timeZone = -7;  // Pacific Daylight Time (USA)

*/

#include "NISTClass.h"
#include "DS1307RTCClass.h"

#include <WiFiConnect.h>
#include <ESP8266WiFi.h>

#define aSSID  "mySSID"
#define aPASSWORD "myPWD"

#define DS1370_I2C_ADDRESS 0x68

//Toggle the comments in the following two lines to operare 
//the sketch as a NIST/NTP clock or a DS1370RTC clock

NISTClass* timeClass = NISTClass::Instance(300);
//DS1307RTCClass* timeClass = DS1307RTCClass::Instance(DS1370_I2C_ADDRESS, D1, D2);

void setup(){
	Serial.begin(115200);
	Serial.println("\n\nTimeClass Example");
	//Comment the following line if this sketch is a DS1370RTC
	WiFiConnect::Instance(aSSID, aPASSWORD)->Connect();	Serial.println(WiFi.localIP());
	//update every 60 seconds. Change this to 600 when you have tested your system.
	timeClass->init(60); 
}

static time_t prevDisplay = 0; // when the digital clock was displayed

void loop(){
	if (timeClass->timeStatus() != timeNotSet) {
		time_t t = timeClass->now();
		if ( t != prevDisplay) { //update the display only if time has changed			
			prevDisplay = t;
			displayTime();			
		}
	}
}

void displayTime(){
	const tmElements_t& tm = timeClass->getTimeElements();
	Serial.print("UTC "); 
	if(tm.Hour < 10)Serial.print("0");Serial.print(tm.Hour); Serial.print(":"); 
	if(tm.Minute < 10)Serial.print("0");Serial.print(tm.Minute); Serial.print(":"); 
	if(tm.Second < 10)Serial.print("0");Serial.print(tm.Second); Serial.print(", ");	
	
	Serial.print(timeClass->dayStr(tm.Wday)); Serial.print(", "); Serial.print(timeClass->monthStr(tm.Month)); 
	Serial.print(" "); Serial.print(tm.Day);  Serial.print(" 20");Serial.println(tm.Year);
}
