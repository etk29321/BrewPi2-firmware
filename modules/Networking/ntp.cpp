/*
 * Code modified from: https://community.particle.io/t/getting-utc-time-from-ntp-server/1213
 *
 */
#include "ntp.h"


bool ntpSync(String ntpserver) {
	UDP *ntpUdp=new UDP();
	IPAddress server=WiFi.resolve(ntpserver);
	bLink->printDebug("Syncing time with ntp server %s at %d.%d.%d.%d",ntpserver,server[0],server[1],server[2],server[3]);
	int elapsed=0;
	int timeOut=10000; //timeout in mS
	int localPort=8888; //local port to listen for ntp packets on
	ntpUdp->begin(localPort); //begin listening
	sendNTPpacket(ntpUdp,server); // send an NTP packet to a time server
	while (!ntpUdp->available() && elapsed<timeOut) { // wait for reply
		delay(10);
		ntpUdp->parsePacket();
		elapsed+=10;  //this doesnt need to be precise, so we'll do this quickly instead of calling millis().
	}
	if ( ntpUdp->available() ) {
		byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
		ntpUdp->read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

		//the timestamp starts at byte 40 of the received packet and is four bytes,
	    // or two words, long. First, esxtract the two words:

	    //unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
	    //unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
	    // combine the four bytes (two words) into a long integer
	    // this is NTP time (seconds since Jan 1 1900):
	    //unsigned long secsSince1900 = highWord << 16 | lowWord;
	    unsigned long secsSince1900 = (packetBuffer[40] << 24) +( packetBuffer[41] << 16) + (packetBuffer[42] << 8) + packetBuffer[43];
    	//bLink->printDebug("secsSince1900=%d",secsSince1900);


	    // now convert NTP time into everyday time:
	    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
	    const unsigned long seventyYears = 2208988800UL;
	    // subtract seventy years:
	    unsigned long epoch = secsSince1900 - seventyYears;
    	//bLink->printDebug("epoch=%d",epoch);

	    if (epoch) {
	    	Time.setTime(epoch);
	    	ntpUdp->stop();
	    	delete ntpUdp;
	    	bLink->printDebug("Time set by ntp server %s to %s",ntpserver,Time.timeStr());
	    	return true;
	    }
	} else {
		bLink->printDebug("Timeout waitng for reply from ntp server (timeout=%d, waited %d ms)",timeOut,elapsed);
	}
	ntpUdp->stop();
	delete ntpUdp;
	bLink->printDebug("Failed to sync time with ntp server %s",ntpserver);
	return false;
}



// send an NTP request to the time server at the given address
void sendNTPpacket(UDP *ntpUdp, IPAddress address) {
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  ntpUdp->sendPacket( packetBuffer,NTP_PACKET_SIZE,  address, 123); //NTP requests are to port 123
}
