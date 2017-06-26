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
    if (configured) {
    	//             1985-04-12T19:20:50.52-04:00
    	//Time.setFormat("%Y-%m-%dT%T%z");
	String *packet = new String(msgtype + SYSLOG_VER + SP + Time.format(Time.now(), "%Y-%m-%dT%T%z") + SP + hname + SP + \
			               SYSLOG_APPNAME + SP + NILVAL + SP + NILVAL + SP + NILVAL + SP + msg);
	//HEADER          = PRI VERSION SP TIMESTAMP SP HOSTNAME
	//                        SP APP-NAME SP PROCID SP MSGID
	//String *packet = new String(PRI + SP+ SYSLOG_VER + SP +TIMESTAMP + SP + hname + SP + \
	//		               SYSLOG_APPNAME + SP + PROCID + SP + MSGID + SP + STRCUTURED_DATA + SP + msg);

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
}

Syslog::Syslog(){
    configured=false;
}

Syslog::Syslog(String hostname, IPAddress syslogServer, int syslogPort){
	hname=hostname;
	server=syslogServer;
	port=syslogPort;
    configured=true;
}

void Syslog::configure(String hostname, IPAddress syslogServer, int syslogPort){
    hname=hostname;
    server=syslogServer;
    port=syslogPort;
    configured=true;
}
