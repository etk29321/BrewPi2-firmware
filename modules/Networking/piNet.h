#ifndef PINET_H
#define PINET_H

#include "application.h"
#include "storage.h"
#include "../mdns/mdns.h"
#include "syslog.h"
#include "json.h"


#define MAX_WIFI_CONNECT_WAIT 30000

class PiNet {
public:
	PiNet();
    bool connect();
    bool disconnect();
    bool validHostname(char *name);
    void processQueries();
    bool setConfig(NetworkBlock *netconfig);
    bool setConfig(JSONObj *json);
    NetworkBlock *getConfig();
    JSONObj *jsonify();
    void log(String msg);
    void log(String msgtype, String msg);
private:
    MDNS mdns;
    Syslog syslog;
    bool useDHCP;
    IPAddress ipAddress;
    IPAddress netmask;
    IPAddress gateway;
    IPAddress dnsServer;
    String hostname; // 63-characters is the max length of a hostname in the mdns lib (label.h)
    bool enableSyslog;
    IPAddress syslogServer;
    uint16_t syslogPort;

};

IPAddress StringToIPAddress(char *str);

extern PiNet piNet;
#endif
