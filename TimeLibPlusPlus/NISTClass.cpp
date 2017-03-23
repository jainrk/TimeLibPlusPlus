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
#include "NISTClass.h"

static 	const unsigned int UDPPort  = 2390;    // Local port to listen for UDP packets


NISTClass* NISTClass::_instance = 0;
const char* _ntpServerName = "us.pool.ntp.org";

NISTClass::NISTClass() {
	Udp.begin(UDPPort);
}

NISTClass* NISTClass::Instance(unsigned int udpDelay = 200) {
	if (_instance == 0)
	_instance = new NISTClass;
	_instance->UDPDelay = udpDelay;
	return _instance;
}

time_t NISTClass::getTimeFromSource(){
	return getRawTimeStarting1900();
}	

unsigned long NISTClass::sendNTPPacket() { 
	memset(NTPPacket, 0, NTP_PACKET_SIZE); // set all bytes in the buffer to 0
	NTPPacket[0] = 0b11100011;   // LI, Version, Mode
	NTPPacket[1] = 0;     // Stratum, or type of clock
	NTPPacket[2] = 6;     // Polling Interval
	NTPPacket[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	NTPPacket[12]  = 49;
	NTPPacket[13]  = 0x4E;
	NTPPacket[14]  = 49;
	NTPPacket[15]  = 52;  


	IPAddress timeServerIP;
	WiFi.hostByName(_ntpServerName, timeServerIP);

	Udp.beginPacket(timeServerIP, 123);
	Udp.write(NTPPacket, NTP_PACKET_SIZE);
	Udp.endPacket();
}

unsigned long NISTClass::getRawTimeStarting1900() { //UNIX Time = SecondsSince1970;
	sendNTPPacket(); // send an NTP packet to a time server
	delay(UDPDelay);

	if (Udp.parsePacket()) {
		Udp.read(NTPPacket, NTP_PACKET_SIZE); // read the packet into the buffer
		unsigned long highWord = word(NTPPacket[40], NTPPacket[41]);
		unsigned long lowWord = word(NTPPacket[42], NTPPacket[43]);
		unsigned long secsSince1900 = highWord << 16 | lowWord;
		return (unsigned long)(secsSince1900);
	} else {
		return 0;
	}
}
