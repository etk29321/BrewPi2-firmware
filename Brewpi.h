#ifndef BREWPI_H
#define BREWPI_H

#pragma once


#ifndef oneWirePin
#define oneWirePin 0x0
#endif

#ifndef actuatorPin0
#define actuatorPin0 A7
#endif

#ifndef actuatorPin1
#define actuatorPin1 A6
#endif

#ifndef actuatorPin2
#define actuatorPin2 A1
#endif

#ifndef actuatorPin3
#define actuatorPin3 A0
#endif

#define WIRING 1


#define PROGMEM
#define PSTR(x) (x)
#define pgm_read_byte(x)  (*(x))


#define PRINTF_PROGMEM "%s"             // devices with unified address space

#define ONEWIRE_DS248X

#include "testing.h"

// forward declarations
class DeviceManager;
class PID;
class Device;
class OneWireGPIO;
class HardwareGPIO;
class OneWireTempSensor;
class PIDs;
class Connection;
class Connections;
class Syslog;
class Storage;
class DEVSTORObj;
class PIDSTORObj;
class CONNSTORObj;
typedef struct deventity;
typedef struct pidentity;
typedef struct connentity;
typedef struct NetworkBlock;

#ifndef XCODE
#include "application.h"
typedef uint8_t DeviceAddress[8];
#include "debugMalloc.h"
#include "BrewLink.h"
#endif

#include "PID.h"
#include "DeviceManager.h"
#include "Connect.h"
//#include "syslog.h"
#include "modules/Networking/piNet.h"
#include "Storage.h"

void setup(void);
void loop (void);

extern class BrewLink *bLink;
extern class DeviceManager *deviceManager;
extern class PIDs *pids;
extern class Connections *connections;
//extern class Syslog syslog;
extern class Storage *storage;




#endif
