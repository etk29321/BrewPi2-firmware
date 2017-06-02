#include "DeviceManager.h"





void OneWireTempSensor::init(DallasTemperature *owbus, uint8_t *addr){ //OneWire Device
	sensor=owbus;
    if (sensor!=NULL && addr!=NULL) {
        address=(uint8_t *)malloc(sizeof(uint8_t)*8);
        memcpy(address,addr,8*sizeof(uint8_t));
        sensor->initConnection(address);
    	//sensor->requestTemperaturesByAddress(address); //tell sensor to read temp
    	//delay(750); //wait for conversion
    }
	CorF=F; //default to Farenheit
	if (name==NULL) { //then set to default name
		name=(char *)malloc(sizeof(char)*17);
        sprintf(name,"%02x%02x%02x%02x%02x%02x%02x%02x",address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7]);
	}
}
OneWireTempSensor::~OneWireTempSensor(){
	if (name!=NULL) {
			free(name);
			name=NULL;
	}
    if (address!=NULL) {
        free(address);
        address=NULL;
    }
	sensor=NULL;
}
double OneWireTempSensor::getValue(){
	if (address==NULL || sensor==NULL) {
		bLink->printDebug("ERROR: OneWireTempSensor::getValue called with NULL address or sensor");
		return -195;
	}
	sensor->requestTemperaturesByAddress(address); //tell sensor to read temp
	//delay(750); //wait for converstion
	switch (CorF) {
	case C:
		return sensor->getTempC(address); //get temp in C
		break;
	case F:
		return sensor->getTempF(address); //get temp in F
		break;
	default:
		return sensor->getTempF(address); //get temp in F
	}
}

uint8_t *OneWireTempSensor::getAddress(){
	return address;
}

bool OneWireTempSensor::matchAddress(uint8_t *addr){
	if(addr!=NULL && address!=NULL) {
		for(int i=0;i<8;i++) {
			if(addr[i]!=address[i]) {
				return false;
			}
		}
	}
	return true;
}

void OneWireTempSensor::setOutput(int value){
	return;
}

void OneWireTempSensor::setCorF(TempFormat newCorF){
	CorF=newCorF;
	return;
}


JSONObj *OneWireTempSensor::jsonify(){ //prints the defintion of this object in JSON format
	JSONObj *json=new JSONObj();
	//bLink->printDebug("OneWire Temp");

	char addbuf[17];
	json->addElement("Name",name);
	json->addElement("DeviceType",devType);
	json->addElement("DeviceHardware",devHardware);
    if (address!=NULL) {
        sprintf(addbuf,"%02x%02x%02x%02x%02x%02x%02x%02x",address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7]);
    } else {
        sprintf(addbuf,"0x0");
    }
	json->addElement("address",addbuf);
	json->addElement("CorF",CorF);
	//json->addElement("value",getValue());  //getValue takes ~500ms so we don't do this unless we're really sure we should
	//json->addElement("isParasitePowered",sensor->isParasitePowered(address));

	return json;
}



/******************************************************************************
 *
 * OneWireGPIO
 *3a0000000
 *****************************************************************************/


void OneWireGPIO::init(OneWire *owbus, uint8_t *addr, OneWireGPIOPIO pionum){ //OneWire 2-channel GPIO Device
	bus=owbus;
    if (bus!=NULL && addr!=NULL) {
        address=(uint8_t *)malloc(sizeof(uint8_t)*8);
        memcpy(address,addr,8*sizeof(uint8_t));
        owSwitch=new DS2413();
        owSwitch->init(owbus,address);
        pio=pionum;
    	if (name==NULL) { //then set to default name
    		name=(char *)malloc(sizeof(char)*18);
            sprintf(name,"%02x%02x%02x%02x%02x%02x%02x%02x%1d",address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7],pio);
    	}
    }
}
OneWireGPIO::~OneWireGPIO(){
	if (name!=NULL) {
			free(name);
			name=NULL;
	}
	if (owSwitch!=NULL) {
		delete owSwitch;
		owSwitch=NULL;
	}
    if (address!=NULL) {
        free(address);
        address=NULL;
    }
    bus=NULL;
}
double OneWireGPIO::getValue(){
	if (owSwitch==NULL) {
		bLink->printDebug("ERROR: OneWireGPIO::getValue called with NULL owSwitch");
		return -1;
	}
	int val=owSwitch->channelReadAll();
	switch (pio){
		case PIOA:
			if (val&0x01) {
				return LOW;
			} else {
				return HIGH;
			}
			break;
		case PIOB:
			if(val>>1)
			{
				return LOW;
			} else {
				return HIGH;
			}
			break;
        case UNSET:
            return -1;
            break;
	}
	return -1;
}
void OneWireGPIO::setOutput(int value){  //OneWire GPIO's are inverted
	if (owSwitch==NULL) {
		bLink->printDebug("ERROR: OneWireGPIO::setOutput called with NULL owSwitch");
		return;
	}
    if(pio==PIOA || pio==PIOB) {  // only run if the PIO number is valid
	if (value==HIGH) {
#ifndef XCODE
		//bLink->printDebug("Setting GPIO output of 0x%02x%02x%02x%02x%02x%02x%02x%02x",address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7]);
#endif
		//String nameStr=charToString(getName());
		//syslog.log("Setting GPIO output " + nameStr + " to LOW");
		char msgbuf[246];
        sprintf(msgbuf,"Setting GPIO output %s to LOW",getName());
        syslog.log(msgbuf);
		owSwitch->channelWrite(pio,LOW);
#ifndef XCODE
       // bLink->printDebug("GPIO output set to %d",value);
#endif
    } else {
#ifndef XCODE
        //bLink->printDebug("Setting GPIO output of 0x%02x%02x%02x%02x%02x%02x%02x%02x",address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7]);
#endif
		//String nameStr=charToString(getName());
		//syslog.log("Setting GPIO output " + nameStr + " to HIGH");
		char msgbuf[246];
        sprintf(msgbuf,"Setting GPIO output %s to HIGH",getName());
        syslog.log(msgbuf);
        owSwitch->channelWrite(pio,HIGH);
#ifndef XCODE
       // bLink->printDebug("GPIO output set to %d",value);
#endif
	}
    }
}
PinMode OneWireGPIO::getGPIOMode(){ // DS2413 doesnt really have a mode setting, so this is provided for compatiblity with hardware GPIO
	return 	gpioMode;
}
void OneWireGPIO::setGPIOMode(PinMode mode){
	gpioMode=mode;
}

uint8_t *OneWireGPIO::getAddress(){
	return address;
}

bool OneWireGPIO::matchAddress(uint8_t *addr){
	if(addr!=NULL && address!=NULL) {
		for(int i=0;i<8;i++) {
			if(addr[i]!=address[i]) {
				return false;
			}
		}
	} else {
		return false;
	}
	return true;
}

OneWireGPIOPIO OneWireGPIO::getPin(){
	return pio;
}

JSONObj *OneWireGPIO::jsonify(){
	JSONObj *json=new JSONObj();
	//bLink->printDebug("OneWire GPIO PIO %d", pio);

	char addbuf[17];
	json->addElement("Name",name);
	json->addElement("DeviceType",devType);
	json->addElement("DeviceHardware",devHardware);
	json->addElement("gpioMode",gpioMode);
    if (address!=NULL) {
        sprintf(addbuf,"%02x%02x%02x%02x%02x%02x%02x%02x",address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7]);
    } else {
        sprintf(addbuf,"0x0B");
    }
	json->addElement("address",addbuf);
	json->addElement("pio",pio);
	//json->addElement("value",getValue());

	return json;
}


/******************************************************************************
 *
 * HardwareGPIO
 *
 *****************************************************************************/
/*HardwareGPIO::HardwareGPIO(DeviceHardware dH) { //hardware GPIO pin
	gpioMode=OUTPUT; //defaults to output
	pin=-1; //hardware pin number
}*/

void HardwareGPIO::init(int pinnum) { //hardware GPIO pin
	gpioMode=OUTPUT; //defaults to output
	pin=pinnum; //hardware pin number
	if (name==NULL) { //then set to default name
		name=(char *)malloc(sizeof(char)*3);
        sprintf(name,"%02d",pin);
	}
}

HardwareGPIO::~HardwareGPIO() { //hardware GPIO pin
	if (name!=NULL) {
			free(name);
			name=NULL;
	}
	return;
}

double HardwareGPIO::getValue(){
	return digitalRead(pin);
}
void HardwareGPIO::setOutput(int value){
	if (value==HIGH || value==LOW) { //validate value is in the correct range
		digitalWrite(pin,value);
	}
}
int HardwareGPIO::getAnalogValue(){
	return analogRead(pin);
}
void HardwareGPIO::setPWM(int value){  //PWM duty cycle (0-255)
	if(value>=0 && value <=255) {
		analogWrite(pin,value);
	}
}
PinMode HardwareGPIO::getGPIOMode(){
	return getPinMode(pin);
}

int HardwareGPIO::getPin(){
	return pin;
}

void HardwareGPIO::setGPIOMode(PinMode mode){
	pinMode(pin,mode);
}

JSONObj *HardwareGPIO::jsonify(){
	JSONObj *json=new JSONObj();
	//bLink->printDebug("GPIO on pin %d", pin);

	json->addElement("Name",name);
	json->addElement("DeviceType",devType);
	json->addElement("DeviceHardware",devHardware);
	json->addElement("gpioMode",gpioMode);
	json->addElement("pin",getPin());
	//json->addElement("value",getValue());
	return json;
}

/******************************************************************************
 *
 * Device
 *
 *****************************************************************************/
Device::Device(uint8_t DevID, DeviceHardware dH){
	DeviceID=DevID;
	devHardware=dH;
	switch(dH) {  //set devType based on devHardware
	case DEVICE_HARDWARE_ONEWIRE_TEMP:
		devType=DEVICETYPE_TEMP_SENSOR;
		break;
	case DEVICE_HARDWARE_PIN:
	case DEVICE_HARDWARE_ONEWIRE_2413:
		devType=DEVICETYPE_GPIO;
		break;
	default:
		devType=DEVICETYPE_NONE;
	}
}

Device::~Device() {
	return;
}

void Device::setName(char *d) {
	if(name!=NULL) {
		free(name);
	}
	size_t namelen=strlen(d)+1;
	name=(char *)malloc(sizeof(char)*namelen);
	memcpy(name,d,namelen);
}

char *Device::getName() {
	return name;
}

void Device::setStrName(String str) {
	strName=str;
}

String Device::getStrName(){
	return strName;
}

DeviceHardware Device::getDeviceHardware() {
	return devHardware;
}

/******************************************************************************
 *
 * DeviceManager
 *
 *****************************************************************************/
DeviceManager::DeviceManager() {
	devices=NULL;
	devCount=0;
	oneWireBus= new OneWire(oneWirePin); //instantiate the OneWire bus
	sensor= new DallasTemperature(oneWireBus); //instantiate the OneWire temp sensor interface
	return;
}

void DeviceManager::addDevice(Device *dev){
	if (devices==NULL) {
		devices=(Device **)malloc(sizeof(Device *));
		devCount=1;
		devices[0]=dev;
		if (dev->getDeviceHardware()==DEVICE_HARDWARE_ONEWIRE_TEMP) { //if this is a new temp sensor, create a PID
	                PID *owpid=new PID(dev,5.00,0.25,1.50,0.5,1);
	                pids->addPID(owpid);
	                JSONObj *json=owpid->jsonify();
	                char *buf=json->jstringify();
	                delete json;
	                free(buf);
	    }
	} else {
		if (!devExists(dev)) {
			Device **newroot=(Device **)realloc(devices,sizeof(Device *)*(devCount+1));
			if (newroot==NULL) {
				return;
			}
			if (newroot!=devices) {
				devices=newroot;
			}
			devices[devCount]=dev;

			if (dev->getDeviceHardware()==DEVICE_HARDWARE_ONEWIRE_TEMP) { //if this is a new temp sensor, create a PID
                PID *owpid=new PID(dev,5.00,0.25,1.50,1,1);
                pids->addPID(owpid);
            }
			devCount++;
		} else {
			delete dev; //already exists.  we don't want to add a dup
		}
	}
}

bool DeviceManager::devExists(Device *dev) {
	HardwareGPIO *a;
	HardwareGPIO *b;
	if(dev!=NULL && devices!=NULL) {
		for (int i=0;i<devCount;i++) {
			if (devices[i]!=NULL) {
				if (devices[i]->getDeviceHardware() == dev->getDeviceHardware()) {
                    bool match;
                    int pina;
                    int pinb;
					switch(dev->getDeviceHardware()) {
					case DEVICE_HARDWARE_ONEWIRE_TEMP:
                            match=((OneWireTempSensor *)(devices[i]))->matchAddress(((OneWireTempSensor *)dev)->getAddress());
                            if (match) {
                                return true;
                            }
						break;
					case DEVICE_HARDWARE_PIN:
							a=(HardwareGPIO *)devices[i];
							b=(HardwareGPIO *)dev;
                            pina=a->getPin();
                            pinb=b->getPin();
                            if (pina==pinb) {
                                return true;
                            }
						break;
					case DEVICE_HARDWARE_ONEWIRE_2413:
                            match= ((OneWireGPIO *)(devices[i]))->matchAddress(((OneWireGPIO *)dev)->getAddress());
                            pina=((OneWireGPIO *)(devices[i]))->getPin();
                            pinb=((OneWireGPIO *)dev)->getPin();
                            if (match && (pina==pinb)) {
                                return true;
                            }
                            
						break;
					default:
						return false;
					}
				}
			}
		}
	}
	return false;
}

Device *DeviceManager::getDevice(char *name){
	if (devices!=NULL && name!=NULL) {
		for (int i=0;i<devCount;i++) {
			if ((devices[i]->getName() != NULL) && (!strcmp(name,devices[i]->getName()))) {
				return devices[i];
			}
		}
	}
	return NULL; //no such device name found
}

Device *DeviceManager::getDevice(int id){
	if (devices!=NULL && id<devCount) {
		return devices[id];
	} else {
		return NULL;
	}
}

Device *DeviceManager::getDevice(uint8_t *addr){
	if (devices!=NULL && addr!=NULL) {
		for (int i=0;i<devCount;i++) {
			switch(devices[i]->getDeviceHardware()) {
				case DEVICE_HARDWARE_ONEWIRE_TEMP:
					if (((OneWireTempSensor *)devices[i])->matchAddress(addr)) {
						return devices[i];
					}
					break;
				case DEVICE_HARDWARE_ONEWIRE_2413:
					if (((OneWireGPIO *)devices[i])->matchAddress(addr)) {
						return devices[i];
					}
					break;
			}
		}
	}
	return NULL; //no such device name found
}

Device *DeviceManager::getDevice(uint8_t *addr, uint8_t pio){
	if (devices!=NULL && addr!=NULL) {
		for (int i=0;i<devCount;i++) {
			switch(devices[i]->getDeviceHardware()) {
				case DEVICE_HARDWARE_ONEWIRE_TEMP:
					if (((OneWireTempSensor *)devices[i])->matchAddress(addr)) {
						return devices[i];
					}
					break;
				case DEVICE_HARDWARE_ONEWIRE_2413:
					if (((OneWireGPIO *)devices[i])->matchAddress(addr) && (((OneWireGPIO *)devices[i])->getPin()==pio)) {
						return devices[i];
					}
					break;
			}
		}
	}
	return NULL; //no such device name found
}

Device *DeviceManager::getHWGPIODevice(int pin){
	if (devices!=NULL) {
		for (int i=0;i<devCount;i++) {
			switch(devices[i]->getDeviceHardware()) {
				case DEVICE_HARDWARE_PIN:
					if (((HardwareGPIO *)devices[i])->getPin()==pin) {
						return devices[i];
					}
					break;

			}
		}
	}
	return NULL; //no such device name found
}

DeviceManager::~DeviceManager() {
	if(devices!=NULL) {
		for(int i=0;i<devCount;i++) {
			if(devices[i]!=NULL) {
				switch(devices[i]->getDeviceHardware()) {
				case DEVICE_HARDWARE_ONEWIRE_TEMP:
					delete (OneWireTempSensor *)devices[i];
					break;
				case DEVICE_HARDWARE_PIN:
					delete (HardwareGPIO *)devices[i];
					break;
				case DEVICE_HARDWARE_ONEWIRE_2413:
					delete (OneWireGPIO *)devices[i];
					break;
				default:
					delete devices[i];
				}
			}
		}
		free(devices);
		devices=NULL;
	}
	return;
}

JSONObj *DeviceManager::jsonify(){
	JSONObj *json=new JSONObj();
	JSONObj *jsonchild=new JSONObj();
	JSONArray *devArray=new JSONArray();
	//bLink->printDebug("Device Manager.  Has %d Devices", devCount);

	for (int i=0;i<devCount;i++) {
        JSONObj *jsonchild;
    	//bLink->printDebug("Device Manager.  Devices #%d", i);

        if (devices[i]!=NULL) {
        switch(devices[i]->getDeviceHardware()) {
            case DEVICE_HARDWARE_ONEWIRE_TEMP:
                jsonchild=((OneWireTempSensor *)(devices[i]))->jsonify();
                devArray->addElement(jsonchild);
                break;
            case DEVICE_HARDWARE_PIN:
                jsonchild=((HardwareGPIO *)(devices[i]))->jsonify();
                devArray->addElement(jsonchild);
                break;
            case DEVICE_HARDWARE_ONEWIRE_2413:
                jsonchild=((OneWireGPIO *)(devices[i]))->jsonify();
                devArray->addElement(jsonchild);
                break;
            case DEVICE_HARDWARE_NONE:
                devArray->addElement("DEVICE_HARDWARE_NONE");
                break;

        }
        } else {
            devArray->addElement("NULL_DEVICE");
        }
	}
	jsonchild->addElement("Devices",devArray);
	jsonchild->addElement("deviceCount",devCount);
	json->addElement("DeviceManager",jsonchild);
	return json;
}

DEVSTORObj *DeviceManager::storeify(){
	DEVSTORObj *store=new DEVSTORObj(devCount);
	store->devCount=devCount;
	for (int i=0;i<devCount;i++) {
        deventity *child=store->devs[i];

        if (devices[i]!=NULL) {
        	child->DeviceID=i;
        	int namelen=strlen(devices[i]->getName());
        	if (namelen>19) {
        		namelen=19;
        	}
        	memcpy(child->Name,devices[i]->getName(),namelen); //copy name, up to 16 chars
        	child->Name[namelen]='\0'; //ensure string is null terminated
			child->CorF=1; //temp format single bit
			child->DeviceHardware=devices[i]->getDeviceHardware(); // enum, 2 bits
			switch(devices[i]->getDeviceHardware()) {
            	case DEVICE_HARDWARE_PIN:
        			child->pinpio=((HardwareGPIO *)devices[i])->getPin(); // gpio 1wire pio or hw pin, enum, 2 bits
        			child->gpioMode=((HardwareGPIO *)devices[i])->getGPIOMode(); //all gpio's, enum, 2 bits
        			break;
            	case DEVICE_HARDWARE_ONEWIRE_2413:
        			child->pinpio=((OneWireGPIO *)devices[i])->getPin(); // gpio 1wire pio or hw pin, enum, 2 bits
        			child->gpioMode=((OneWireGPIO *)devices[i])->getGPIOMode(); //all gpio's, enum, 2 bits
                	memcpy(child->address,((OneWireGPIO *)devices[i])->getAddress(),8); //copy one-wire address
        			break;
            	case DEVICE_HARDWARE_ONEWIRE_TEMP:
                	memcpy(child->address,((OneWireTempSensor *)devices[i])->getAddress(),8); //copy one-wire address
            	default:
        			child->pinpio=2; // gpio 1wire pio or hw pin, enum, 2 bits
        			child->gpioMode=0; //all gpio's, enum, 2 bits
            		break;
			}
        } else {
           	child->DeviceID=i;
            	memcpy(child->Name,"UNKNOWN",7); //copy name, up to 16 chars
            	child->Name[7]='\0';
            	memcpy(child->address,"00000000",8); //copy one-wire address
    			child->CorF=1; //temp format single bit
    			child->DeviceHardware=0; // enum, 2 bits
            	child->pinpio=2; // gpio 1wire pio or hw pin, enum, 2 bits
            	child->gpioMode=0; //all gpio's, enum, 2 bits
        }
    }
	return store;
}

JSONObj *DeviceManager::status(){
	JSONObj *json=new JSONObj();

	for (int i=0;i<devCount;i++) {
        if (devices[i]!=NULL && devices[i]->getName()!=NULL) {
            json->addElement(devices[i]->getName(),devices[i]->getValue());
        } else {
            json->addElement("NULL_DEVICE",(double)0);
        }
	}
	return json;
}

/***********************************************************************
**
** Function: DeviceManager::deviceSearch
** Purpose: Returns a JSON document holding all discovered devices
**
***********************************************************************/
char *DeviceManager::deviceSearch() {
	JSONObj *json;
	OneWireTempSensor *temp;
	OneWireGPIO *owgpio;
	HardwareGPIO *hwgpio;
	char *retbuf;
	oneWireBus->reset_search(); //ensure we start at the beginning
	uint8_t address[8]; //addresses come in 8-byte arrays
	int i=0;
	while (oneWireBus->search(address)) {
		switch (address[0]) {
		case DS18B20MODEL:
		case DS1822MODEL:
		case DS1825MODEL:
			bLink->printDebug("Found OneWire Temp Sensor");

			temp=new OneWireTempSensor(i);
			temp->init(sensor,address);
			//temp->setName("Temp");
			//json=temp->jsonify();
			//retbuf=json->stringify();
			//bLink->printDebug("Details: %s",retbuf);
			//delete json;
			//free(retbuf);
			addDevice(temp);
			break;
		case DS2413_FAMILY_ID:
			owgpio=new OneWireGPIO(i);
			owgpio->init(oneWireBus,address,PIOA);
			//owgpio->setName("OneWire PIOA");

			//json=owgpio->jsonify();
			//retbuf=json->stringify();
			//bLink->printDebug("Details: %s",retbuf);
			//delete json;
			//free(retbuf);

			addDevice(owgpio);
			owgpio=new OneWireGPIO(i);
			owgpio->init(oneWireBus,address,PIOB);
			//owgpio->setName("OneWire PIOB");

			//json=owgpio->jsonify();
			//retbuf=json->stringify();
			//bLink->printDebug("Details: %s",retbuf);
			//delete json;
			//free(retbuf);

			addDevice(owgpio);
			break;
		}
		i++;
	}

	bLink->printDebug("Adding hardware GPIO");
delay(1000);
	// Hardware GPIOs.  These are predefined for Photon
	hwgpio=new HardwareGPIO(i);
	hwgpio->init(actuatorPin0);
	bLink->printDebug("added PIN %d",hwgpio->getPin());
	//hwgpio->setName("actuatorPin0");
	addDevice(hwgpio);

	hwgpio=new HardwareGPIO(i);
	hwgpio->init(actuatorPin1);
	bLink->printDebug("added PIN %d",hwgpio->getPin());
	//hwgpio->setName("actuatorPin1");
	addDevice(hwgpio);

	hwgpio=new HardwareGPIO(i);
	hwgpio->init(actuatorPin2);
	bLink->printDebug("added PIN %d",hwgpio->getPin());
	//hwgpio->setName("actuatorPin2");
	addDevice(hwgpio);

	hwgpio=new HardwareGPIO(i);
	hwgpio->init(actuatorPin3);
	bLink->printDebug("added PIN %d",hwgpio->getPin());
	//hwgpio->setName("actuatorPin3");
	addDevice(hwgpio);


    
	//bLink->printDebug("JSONifying");

	json=jsonify();
	//bLink->printDebug("Stringifying");

	retbuf=json->jstringify();
	//bLink->printDebug("%s",retbuf);

	//delay(2000);

	//bLink->printDebug("Doing cleanup");

	delete json;
	json=NULL;
	return retbuf;
}


