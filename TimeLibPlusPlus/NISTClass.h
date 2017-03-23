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


#ifndef _NISTCLASS_
#define _NISTCLASS_ 1

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "TimeClass.h"

#define NTP_PACKET_SIZE 48

class NISTClass : public TimeClass {
protected:
	NISTClass();
private:
	unsigned long sendNTPPacket();
	unsigned int UDPDelay; 
	byte NTPPacket[NTP_PACKET_SIZE];
	WiFiUDP Udp;
	static NISTClass* _instance;
public:
	static NISTClass* Instance(unsigned int udpDelay);
	unsigned long getRawTimeStarting1900();
	virtual time_t getTimeFromSource();
};

#endif //_NISTCLASS_
