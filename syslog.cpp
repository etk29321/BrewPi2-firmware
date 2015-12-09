#include "syslog.h"


void Syslog::log(char *msg){
	String *strmsg=new String(msg);
	log(SYSLOG_DEBUG,*strmsg);
	delete strmsg;
}


void Syslog::log(String msg){
	log(SYSLOG_DEBUG,msg);
}

void Syslog::log(String msgtype, String msg){

	String *packet = new String(msgtype + SP+ SYSLOG_VER + SP +NILVAL + SP + hname + SP + \
			               SYSLOG_APPNAME + SP + NILVAL + SP + NILVAL + SP + NILVAL + SP + msg);

	packet->toCharArray(buf,480);
	int pktsize=packet->length();
	delete packet;
	if (pktsize>480) {
		pktsize=480;
	}
	udp.begin(port);
	udp.sendPacket(buf, pktsize, server, port);
	udp.stop();
}


Syslog::Syslog(String hostname, IPAddress syslogServer, int syslogPort){
	hname=hostname;
	server=syslogServer;
	port=syslogPort;
}

