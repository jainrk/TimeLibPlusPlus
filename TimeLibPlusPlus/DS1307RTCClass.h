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

#ifndef _DS1307RTCCLASS_
#define _DS1307RTCCLASS_
#include <Wire.h>
#include "TimeClass.h"


class DS1307RTCClass  : public TimeClass {
protected:
	DS1307RTCClass();
private:
	tmElements_t* rtc_packet;
	tmElements_t* rtc_mask;
	byte decToBcd(byte val);
	byte bcdToDec(byte val);

	void readTimeFromDS1370();
	static DS1307RTCClass* _instance;
	byte i2c_address;

public:
	static DS1307RTCClass* Instance(byte _i2c_address, uint8_t sda, uint8_t scl);
	void writeTimeToDS1370(tmElements_t* _rtc_packet);
	//void init(uint8_t sda, uint8_t scl, time_t _syncInterval);	
	virtual time_t getTimeFromSource();
};

#endif //_DS1307RTCClassCLASS_
