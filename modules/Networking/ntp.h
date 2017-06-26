#ifndef NTP_H
#define NTP_H
#include "application.h"
#include "Brewpi.h"

bool ntpSync(String ntpserver);
void sendNTPpacket(UDP *ntpUdp, IPAddress address);

const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

#endif
