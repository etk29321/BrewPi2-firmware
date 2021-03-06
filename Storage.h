#ifndef STORAGE_H
#define STORAGE_H

#include "Brewpi.h"
#include "JSON.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




// photon EEPROM is 2047 bytes
// spark is 127 bytes
///size_t length = EEPROM.length();


typedef struct fstable {
	uint8_t devCount;
	uint8_t pidCount;
	uint8_t connCount;
} fstable;




typedef struct deventity {
	uint8_t DeviceID;
	char Name[20];
	uint8_t address[8]; // 1wire address, 8 bits
	uint8_t CorF; //temp format single bit
	uint8_t DeviceHardware; // enum, 2 bits
	uint8_t pinpio; // gpio 1wire pio or hw pin, enum, 2 bits
	uint8_t gpioMode; //all gpio's, enum, 2 bits
} deventity; //33 bytes per dev (26min)  338 total  - 3 per io board    g, f1, f2  = 348


class DEVSTORObj {
public:
	DEVSTORObj(int count);
	~DEVSTORObj();
	int devCount;
	deventity **devs;
};

typedef struct pidentity {
	uint8_t DeviceID; //deventity name
	double p;
	double i;
	double d;
	double setPoint;
	unsigned long minStateTimeSecs; // in seconds
	double deadBand;
	double PWMScale;
} pidentity; //8 bytes per pid 24 total - 3 PIDs


class PIDSTORObj {
public:
	PIDSTORObj(int Count);
	~PIDSTORObj();
	int pidCount;
	pidentity **pids;
};


typedef struct connentity {
	uint8_t outdevID;
	uint8_t mode;
	uint8_t indevID;
	uint8_t inPIDdevID;
	uint8_t Pstate;
	uint16_t exp; //address of expression string
	uint8_t explen; //length of the exp string
} connentity; //8 bytes per connection 30 total


class CONNSTORObj {
public:
	CONNSTORObj(int count);
	~CONNSTORObj();
	int connCount;
	connentity **conns;
};

class Storage {
public:
	Storage();
	~Storage();
	char *read();
	char *write();
	void dump(); //dump EEPROM to debug console
	void clear();
	int writeString(char *str, int pos);
	char *readString(int pos, int len);
	void apply(DEVSTORObj *devstore, PIDSTORObj *pidstore, CONNSTORObj *connstore);

private:
	size_t length;
	int pos;
	int strPos;
};

/*
[CFermenter1 | [CFermenter2
27 bytes



"PIDs":[
{"Sensor":"Fermenter1","Kp":5.00,"Ki":0.25,"Kd":1.50,"setPoint":52.00,"temp":55.96,"PID":0.00,"state":0.00,"lastStateChange":0.00,"minStateTime":120000.00,"stateTime":0.00,"heating":0.00,"cooling":0.00,"deadBand":0.50,"PWMScale":1.00,"PWMDutyCycle":0.00},
{"Sensor":"Glycol","Kp":5.00,"Ki":0.25,"Kd":1.50,"setPoint":40.00,"temp":43.47,"PID":0.00,"state":0.00,"lastStateChange":0.00,"minStateTime":120000.00,"stateTime":0.00,"heating":0.00,"cooling":0.00,"deadBand":1.00,"PWMScale":1.00,"PWMDutyCycle":0.00}
{"Sensor":"Fermenter2","Kp":5.00,"Ki":0.25,"Kd":1.50,"setPoint":52.00,"temp":53.38,"PID":0.00,"state":0.00,"lastStateChange":0.00,"minStateTime":120000.00,"stateTime":1850679396.00,"heating":0.00,"cooling":0.00,"deadBand":1.00,"PWMScale":1.00,"PWMDutyCycle":0.00}]}

[{"OutputDevice":"GlycolChiller","mode":0.00,"PID":"Glycol"},
 {"OutputDevice":"GlycolPump","mode":3.00,"Expression":"[CFermenter1 | [CFermenter2"},
 {"OutputDevice":"HeatingFermenter2","mode":1.00,"PID":"Fermenter2"},
 {"OutputDevice":"GlycolFermenter2","mode":0.00,"PID":"Fermenter2"},
 {"OutputDevice":"HeatingFermenter1","mode":1.00,"PID":"Fermenter1"},
 {"OutputDevice":"GlycolFermenter1","mode":0.00,"PID":"Fermenter1"}]






 {"Connections":[{"OutputDevice":"GlycolChiller","mode":0.00,"PID":"Glycol"},
				 {"OutputDevice":"GlycolPump","mode":3.00,"Expression":"[CFermenter1 | [CFermenter2"},
				 {"OutputDevice":"HeatingFermenter1","mode":1.00,"PID":"Fermenter1"},
				 {"OutputDevice":"GlycolFermenter1","mode":0.00,"PID":"Fermenter1"}]}

				 */

typedef struct NetworkBlock {
    uint8_t useDHCP;
    uint32_t ipAddress;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dnsServer;
    char hostname[64]; // 63-characters is the max length of a hostname in the mdns lib (label.h)
    char ntpserver[64]; // 63-characters is the max length of a hostname in the mdns lib (label.h)
    uint8_t enableSyslog;
    uint32_t syslogServer;
    uint16_t syslogPort;
} NetworkBlock;

#endif
