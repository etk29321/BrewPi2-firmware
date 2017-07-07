#include "piNet.h"
#include <string.h>


PiNet::PiNet() {
    useDHCP=1;
    ntpServer=String("pool.ntp.org"); // 63-characters is the max length of a hostname in the mdns lib (label.h)
    hostname=String("brewpi"); // 63-characters is the max length of a hostname in the mdns lib (label.h)
    enableSyslog=0;
	connected=false;
	return;
}

bool PiNet::connect() {
    //load WIFI
    bLink->printDebug("configuring networking");
    delay(1000);
    if (!useDHCP){
        WiFi.setStaticIP(ipAddress, netmask, gateway, dnsServer);
        WiFi.useStaticIP();
    }
    WiFi.on();
    WiFi.connect();
    int lastConnectAttempt=millis();
    while (millis()-lastConnectAttempt < MAX_WIFI_CONNECT_WAIT && !WiFi.ready()) { // Give WiFi up to 30 seconds to connect before we give up on it.
            delay(100);
    }
    if (WiFi.ready()) {
    	// start bLink TCP
    	bLink->begin();
        // start syslog
        if (enableSyslog) {
            syslog.configure(hostname,syslogServer,syslogPort);
        }
        //start MDNS
        bool success = mdns.setHostname(hostname);
        if (success) {
        	bLink->printDebug("mDNS: set hostname");
            success = mdns.setService("tcp", "brewpi", 8080, hostname);
        }
        if (success) {
        	bLink->printDebug("mDNS: set service");
            success = mdns.begin();
        }
        if(success){
        	bLink->printDebug("mDNS: begun");
        }
        ntpSync(ntpServer); //rsyslog on linux doesnt like syslog messages without valid timestamps
    }
    connected=true;
    return WiFi.ready();
}

bool PiNet::disconnect() {

    bLink->printDebug("stopping mDNS");
    mdns.stop();
    bLink->printDebug("stopping bLink TCP");
    bLink->stop();
    connected=false;
    return true;
}


bool PiNet::validHostname(char *name){
	bLink->printDebug("validating hostname %s",name);
    if (*name=='\0') { // empty string is not a valid name
        return false;
    }
    for (unsigned int pos=0;pos<64;pos++) {
        if (name[pos]=='\0') {
        	bLink->printDebug("hostname %s is valid",name);
            return true;
        }
        if ((name[pos]<'a' || name[pos] > 'z') && (name[pos]<'A' || name[pos] > 'Z') && (name[pos]!='.') && (name[pos] != '-')) { // [a-zA-Z\.\-]
        	bLink->printDebug("invalid char %c in hostname %s",name[pos], name);
        	return false;
        }

    }
	bLink->printDebug("hostname %s exceeded 63 chars",name);
    return false;  // if we got here, the string was not terminated properly
}

void PiNet::processQueries(){
    mdns.processQueries();
}

bool PiNet::setConfig(NetworkBlock *config){
	bool ok;
	bLink->printDebug("reading piNet config");
    //delay(1000);
    if (config!=NULL) {
        useDHCP=config->useDHCP;
        ipAddress=IPAddress(config->ipAddress);
        netmask=IPAddress(config->netmask);
        gateway=IPAddress(config->gateway);
        dnsServer=IPAddress(config->dnsServer);
        if (config->hostname!=NULL && validHostname(config->hostname)) {
            hostname=String(config->hostname);
        } else {
            hostname=System.deviceID();
        }
        if(config->ntpserver[0]!='\0') {
        	ntpServer=String(config->ntpserver);
        } else {
        	ntpServer=String("");
        }
        enableSyslog=config->enableSyslog;
        syslogServer=IPAddress(config->syslogServer);
        syslogPort=config->syslogPort;
    } else {
        useDHCP=true;
        hostname=System.deviceID();
        enableSyslog=false;
    }
    //disconnect();
    //connect();
    return ok;
}


NetworkBlock *PiNet::getConfig(){
	NetworkBlock *config;
	config=(NetworkBlock *)malloc(sizeof(NetworkBlock));
    config->useDHCP=useDHCP;
    config->ipAddress=IPAddressToUint(ipAddress);
    bLink->printDebug("%d.%d.%d.%d = %x",ipAddress[0],ipAddress[1],ipAddress[2],ipAddress[3],config->ipAddress);
    config->netmask=IPAddressToUint(netmask);
    config->gateway=IPAddressToUint(gateway);
    config->dnsServer=IPAddressToUint(dnsServer);
    ntpServer.toCharArray(config->ntpserver,63);
    hostname.toCharArray(config->hostname,63);
    config->enableSyslog=enableSyslog;
    config->syslogServer=IPAddressToUint(syslogServer);
    config->syslogPort=syslogPort;
    return config;
}

JSONObj *PiNet::jsonify(){
	JSONObj *json=new JSONObj();
	json->addElement("useDHCP",useDHCP);
	char buf[16];
	//sprintf(buf,"%d.%d.%d.%d",ipAddress[0]);
	sprintf(buf,"%d.%d.%d.%d",ipAddress[0],ipAddress[1],ipAddress[2],ipAddress[3]);
	json->addElement("ipAddress",buf);
	sprintf(buf,"%d.%d.%d.%d",netmask[0],netmask[1],netmask[2],netmask[3]);
	json->addElement("netmask",buf);
	sprintf(buf,"%d.%d.%d.%d",gateway[0],gateway[1],gateway[2],gateway[3]);
	json->addElement("gateway",buf);
	sprintf(buf,"%d.%d.%d.%d",dnsServer[0],dnsServer[1],dnsServer[2],dnsServer[3]);
	json->addElement("dnsServer",buf);
	char ntpsvr[64];
	ntpServer.toCharArray(ntpsvr,63);
	json->addElement("ntpserver",ntpsvr);
	char hname[64];
	hostname.toCharArray(hname,63);
	json->addElement("hostname",hname);
	json->addElement("enableSyslog",enableSyslog);
	sprintf(buf,"%d.%d.%d.%d",syslogServer[0],syslogServer[1],syslogServer[2],syslogServer[3]);
	json->addElement("syslogServer",buf);
	json->addElement("syslogPort",syslogPort);
	return json;
}

//JSONObj *json=new JSONObj(jsontxt);
//JSONElement *jsonOutDev=json->getElement("useDHCP");
bool PiNet::setConfig(JSONObj *json){
	bool ok;
	JSONElement *jsonel;
	bLink->printDebug("reading piNet config");
    if (json!=NULL) {
        jsonel=json->getElement("useDHCP");
        if(jsonel!=NULL){
        	useDHCP=(int)jsonel->getValueAsDouble();
        }
        jsonel=json->getElement("ipAddress");
        if(jsonel!=NULL){
        	ipAddress=StringToIPAddress(jsonel->getValue());
        	bLink->printDebug("set IP address to %d.%d.%d.%d",ipAddress[0],ipAddress[1],ipAddress[2],ipAddress[3]);
        }
        jsonel=json->getElement("netmask");
        if(jsonel!=NULL){
        	netmask=StringToIPAddress(jsonel->getValue());
        }
        jsonel=json->getElement("gateway");
        if(jsonel!=NULL){
        	gateway=StringToIPAddress(jsonel->getValue());
        }
        jsonel=json->getElement("dnsServer");
        if(jsonel!=NULL){
        	dnsServer=StringToIPAddress(jsonel->getValue());
        }
        jsonel=json->getElement("hostname");
        if(jsonel!=NULL){
        	if (jsonel->getValue()!=NULL && validHostname(jsonel->getValue())) {
        		hostname=String(jsonel->getValue());
        	} else {
        		hostname=System.deviceID();
        	}
        } else {
            hostname=System.deviceID();
        }
        jsonel=json->getElement("enableSyslog");
        if(jsonel!=NULL){
        	enableSyslog=(int)jsonel->getValueAsDouble();
        }
        jsonel=json->getElement("syslogServer");
        if(jsonel!=NULL){
        	syslogServer=StringToIPAddress(jsonel->getValue());
        }
        jsonel=json->getElement("syslogPort");
        if(jsonel!=NULL){
        	syslogPort=(int)jsonel->getValueAsDouble();
        }
    } else {
        useDHCP=true;
        hostname=System.deviceID();
        enableSyslog=false;
    }
    //disconnect();
    //connect();
    return ok;
}

/*void PiNet::updateConfig(PiNet *newObj){
    NetworkBlock *newConf=newObj->getConfig();
    bool changed=false;
    if (newConf->useDHCP!=config.useDHCP) {
        config.useDHCP=newConf->useDHCP;
        useDHCP=config.useDHCP;
        changed=true;

    }
    if (newConf->ipAddress!=config.ipAddress) {
        config.ipAddress=newConf->ipAddress;
        ipAddress=IPAddress(config.ipAddress);
        changed=true;

    }
    if (newConf->netmask!=config.netmask) {
        config.netmask=newConf->netmask;
        netmask=IPAddress(config.netmask);
        changed=true;

    }
    if (newConf->gateway!=config.gateway) {
        config.gateway=newConf->gateway;
        gateway=IPAddress(config.gateway);
        changed=true;

    }
    if (newConf->dnsServer!=config.dnsServer) {
        config.dnsServer=newConf->dnsServer;
        dnsServer=IPAddress(config.dnsServer);
        changed=true;

    }
    if (!strcmp(newConf->hostname,config.hostname)) {  //hostname
        memcpy(config.hostname,newConf->hostname,64);
        if (validHostname(config.hostname)) {
            hostname=String(config.hostname);
        } else {
            hostname=System.deviceID();
        }
        changed=true;

    }
    if (newConf->enableSyslog!=config.enableSyslog) {
        config.enableSyslog=newConf->enableSyslog;
        enableSyslog=config.enableSyslog;
        changed=true;

    }
    if (newConf->syslogServer!=config.syslogServer) {
        config.syslogServer=newConf->syslogServer;
        syslogServer=IPAddress(config.syslogServer);
        changed=true;

    }
    if (newConf->syslogPort!=config.syslogPort) {
        config.syslogPort=newConf->syslogPort;
        syslogPort=config.syslogPort;
        changed=true;

    }
    if (changed) {
        writeConfig();
    }
}*/


void PiNet::log(String msg){
    if (enableSyslog)
        syslog.log(msg);
}
void PiNet::log(String msgtype, String msg){
    if (enableSyslog)
        syslog.log(msgtype,msg);
}

IPAddress StringToIPAddress(char *str){
	uint8_t ip[4];
	char *pos;
	int octet=0;
	char buf[4];
	char *bufpos=buf;
	if (str!=NULL){
		pos=str;
		while (*pos!='\0' && octet<4) {
			switch (*pos){
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				*bufpos=*pos;
				bufpos++;
				break;
			case '.':
				*bufpos='\0';
				bufpos=buf;
				ip[octet]=atoi(buf);
	        	bLink->printDebug("Parse IP: octet:%d value:%d remaining:|%s|",octet,ip[octet],pos);

				octet++;
				break;
			default:
				//invalid char, parse failed
				return IPAddress(0,0,0,0);
				break;
			}
			pos++;
		}
		*bufpos='\0';
		ip[octet]=atoi(buf);//store last octet

	}
	return IPAddress(ip[0],ip[1],ip[2],ip[3]);
}

uint32_t IPAddressToUint(IPAddress ipAddress){
    return (ipAddress[0]<<24) + (ipAddress[1]<<16) + (ipAddress[2]<<8) + ipAddress[3];
}

