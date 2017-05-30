#ifndef STORAGE_H
#define STORAGE_H

#include "Brewpi.h"
#include "JSON.h"




// photon EEPROM is 2047 bytes
// spark is 127 bytes
///size_t length = EEPROM.length();


struct fstable {
	uint8_t devCount,
	uint8_t pidCount,
	uint8_t connCount
}

typedef struct fstable

struct deventity {
	uint8_t DeviceID,
	char Name[16],
	uint8_t address[8], // 1wire address, 8 bits
	uint8_t CorF, //temp format single bit
	uint8_t DeviceType, // enum, 2 bits
	uint8_t DeviceHardware, // enum, 2 bits
	uint8_t pinpio // gpio 1wire pio or hw pin, enum, 2 bits
	uint8_t gpioMode, //all gpio's, enum, 2 bits
}; //29 bytes per dev (26min)  338 total  - 3 per io board    g, f1, f2  = 348

typedef struct deventity;

class DEVSTORObj {
public:
	DEVSTORObj();
	~DEVSTORObj();
	int devCount;
	deventity **devs;
};

struct pidentity {
	uint8_t DeviceID, //deventity name
    uint8_t p,
    uint8_t i,
    uint8_t d,
    uint8_t setPoint,
    uint8_t minStateTimeSecs, // in seconds
    uint8_t deadBand,
    uint8_t PWMScale,
}; //8 bytes per pid 24 total - 3 PIDs

typedef struct pidentity;

class PIDSTORObj {
public:
	PIDSTORObj();
	~PIDSTORObj();
	int pidCount;
	pidentity **pids;
};


struct connentity {
	uint8_t outdevID,
	unit8_t indevID,
	unit8_t inPIDdevID,
	uint8_t Pstate,
	uint8_t exp, //address of expression string
	uint8_t expLen //length of the exp string
}; //5 bytes per connection 30 total

typedef struct connentity;

class CONNSTORObj {
public:
	CONNSTORObj();
	~CONNSTORObj();
	int connCount;
	connentity **conns;
};

class Storage {
public:
	Storage();
	~Storage();
	void read();
	void write();
	int writeString(char *str, int pos);
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


#endif
