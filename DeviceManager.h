#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "Brewpi.h"

#ifdef XCODE
#include "OneWireTest.h"
#else
#include "modules/OneWire/OneWire.h"
#endif
#include "modules/DS2413/DS2413.h"
#include "modules/OneWire/DallasTemperature.h"
//#include "TemperatureFormats.h"
#include "JSON.h"
#include "Storage.h"

#pragma once


enum DeviceType {
	DEVICETYPE_NONE=0,
	DEVICETYPE_TEMP_SENSOR=1,		/* temp sensor */
	DEVICETYPE_GPIO=2,		/* input or output pin */
};

/*enum PinMode { // from pinmap_hal.h
	INPUT,
	OUTPUT,
	INPUT_PULLUP,
	INPUT_PULLDOWN,
};*/

enum OneWireGPIOPIO {
	PIOA=0,
	PIOB=1,
	UNSET=2,
};

enum TempFormat {
	C=0,
	F=1,
};

enum DeviceHardware {
	DEVICE_HARDWARE_NONE=0,
	DEVICE_HARDWARE_PIN=1,			// a digital pin, either input or output
	DEVICE_HARDWARE_ONEWIRE_TEMP=2,         // a onewire temperature sensor
	DEVICE_HARDWARE_ONEWIRE_2413=3          // a onewire 2-channel PIO input or output.
};

class Device {
public:
	Device(uint8_t DevID, DeviceHardware dH);
	virtual ~Device();
	void setName(char *n);
	char *getName();
	void setStrName(String str);
	String getStrName();
	virtual double getValue()=0;
	virtual void setOutput(int)=0;
	DeviceType getDeviceType();
	DeviceHardware getDeviceHardware();
	uint8_t DeviceID;
protected:
	DeviceType devType;
	DeviceHardware devHardware;
	char *name; //human readable name for this device
	String strName;
};


class OneWireTempSensor: public Device {
public:
	OneWireTempSensor(uint8_t DevID, DeviceHardware dH=DEVICE_HARDWARE_ONEWIRE_TEMP): Device(DEVICE_HARDWARE_ONEWIRE_TEMP){
		sensor=NULL;
		CorF=F; //default to Farenheit
        address=NULL;
        name=NULL;
        strName=String("NULL");
	}; //OneWire Device
	void init(DallasTemperature *owbus, uint8_t *address); //OneWire Device
	~OneWireTempSensor();
	double getValue();
	void setOutput(int value);
	JSONObj *jsonify();
	uint8_t *getAddress();
	bool matchAddress(uint8_t *addr);
protected:
	uint8_t *address; //OneWire device address
	DallasTemperature *sensor;
	TempFormat CorF; // C=0, F=1
};

class OneWireGPIO: public Device {
public:
	OneWireGPIO(uint8_t DevID, DeviceHardware dH=DEVICE_HARDWARE_ONEWIRE_2413): Device(DEVICE_HARDWARE_ONEWIRE_2413){
		gpioMode=OUTPUT; //defaults to output
		bus=NULL;
        address=NULL;
        name=NULL;
        strName=String("NULL");
        pio=UNSET;
	}; //OneWire 2-channel GPIO Device
	void init(OneWire *owbus, uint8_t *addr, OneWireGPIOPIO pionum); //OneWire 2-channel GPIO Device
	~OneWireGPIO();
	double getValue();
	void setOutput(int value);
	PinMode getGPIOMode();
	void setGPIOMode(PinMode mode);
	JSONObj *jsonify();
	uint8_t *getAddress();
	OneWireGPIOPIO getPin();
	bool matchAddress(uint8_t *addr);
protected:
	PinMode gpioMode;
	uint8_t *address; //OneWire device address
	OneWireGPIOPIO pio;  //PIOA or PIOB for OneWire DS2413
	DS2413 *owSwitch;
	OneWire *bus;
};

class HardwareGPIO: public Device {
public:
	HardwareGPIO(uint8_t DevID, DeviceHardware dH=DEVICE_HARDWARE_PIN): Device(DEVICE_HARDWARE_PIN){
		gpioMode=OUTPUT; //defaults to output
		pin=-1;
        name=NULL;
        strName=String("NULL");
	}; //hardware GPIO pin
	void init(int pinnum); //hardware GPIO pin
	~HardwareGPIO();
	double getValue();
	void setOutput(int value);
	int getAnalogValue();
	void setPWM(int value);  //PWM duty cycle (0-255)
	PinMode getGPIOMode();
	void setGPIOMode(PinMode mode);
	JSONObj *jsonify();
	int getPin();
protected:
	PinMode gpioMode;
	int pin; //hardware pin number
};




class DeviceManager {
public:
	DeviceManager();
	~DeviceManager();
	void addDevice(Device *dev);
	Device *getDevice(char *name);
	Device *getDevice(int id);
	char *deviceSearch();
	JSONObj *jsonify();
	JSONObj *status();
	bool devExists(Device *dev);
	DEVSTORObj *storeify();

private:
	Device **devices;
	int devCount;
	OneWire *oneWireBus;
	DallasTemperature *sensor;
};


#endif
