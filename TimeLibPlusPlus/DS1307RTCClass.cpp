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

#include <WiFiConnect.h>
#include "DS1307RTCClass.h"

DS1307RTCClass* DS1307RTCClass::_instance = 0;

DS1307RTCClass::DS1307RTCClass() {}

DS1307RTCClass* DS1307RTCClass::Instance(byte _i2c_address, uint8_t sda, uint8_t scl) {
	if (_instance == 0)
	_instance = new DS1307RTCClass;
	_instance->i2c_address = _i2c_address;
	_instance->rtc_packet = new tmElements_t;
	_instance->rtc_mask = new tmElements_t {0x7f,  0x7f,   0x3f, 0x7, 0x3f, 0x1f,  0xff};
	Wire.begin(sda, scl);
	return _instance;
}

byte DS1307RTCClass::decToBcd(byte val) {
	return ((val / 10 * 16) + (val % 10));
}

byte DS1307RTCClass::bcdToDec(byte val) {
	return ((val / 16 * 10) + (val % 16));
}

void DS1307RTCClass::writeTimeToDS1370(tmElements_t* _rtc_packet) {
	// sets time and date data on DS1370
	Wire.beginTransmission(i2c_address);
	Wire.write(0);
	uint8_t* packet_ptr = (uint8_t*)_rtc_packet;
	for (int i = 0; i < 7; i++)
	Wire.write(decToBcd(*packet_ptr++));
	Wire.endTransmission();
}

void DS1307RTCClass::readTimeFromDS1370() {
	Wire.beginTransmission(i2c_address);
	Wire.write(0); // set DS1370 register pointer to 00h
	Wire.endTransmission();
	Wire.requestFrom(i2c_address, 7);
	uint8_t* packet_ptr = (uint8_t*)rtc_packet; 
	uint8_t* mask_ptr = (uint8_t*)rtc_mask; 
	// request seven bytes of data from DS1370 starting at register 00h
	for (int i = 0; i < 7; i++)   
	*packet_ptr++ = (uint8_t)(bcdToDec(Wire.read())&(*mask_ptr++));
}

time_t DS1307RTCClass::getTimeFromSource(){
	readTimeFromDS1370();
	return makeTime(*rtc_packet, 100);	
}	
/* 
void DS1307RTCClass::init(uint8_t sda, uint8_t scl, time_t _syncInterval){
	TimeClass::init(_syncInterval);
}

 */