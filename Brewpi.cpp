#include "Brewpi.h"

//SerialDebugOutput debugOutput(38400, ALL_LEVEL); // use a faster baudrate and log only warnings or more severe


BrewLink *bLink;
DeviceManager *deviceManager;
PIDs *pids;
Connections *connections;
unsigned long lastTime;

void setup()
{
	//we want wifi but no particle cloud
	//SYSTEM_MODE(SEMI_AUTOMATIC);
    //load WIFI
    WiFi.on();
    WiFi.connect();
    bLink=new BrewLink();
    bLink->begin();
    pids=new PIDs();
    connections=new Connections();
    deviceManager=new DeviceManager();
    char * reply=deviceManager->deviceSearch();  //autodiscover any attached devices
    free(reply);

	//  Add some connections
    OneWireTempSensor *temp=(OneWireTempSensor *)deviceManager->getDevice("28fae0a006000001");
	temp->setName("Fermenter1");
    temp=(OneWireTempSensor *)deviceManager->getDevice("287ea2a20600005a");
	temp->setName("Fermenter2");
    PID *temppid=pids->getPID("Fermenter1");
    OneWireGPIO *owgpio=(OneWireGPIO *)deviceManager->getDevice("3af51321000000cb0");
    owgpio->setName("GlycolFermenter1");
    if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed

        Connection *conn=new Connection((Device *)owgpio,temppid,COOLING);
        connections->addConnection(conn);
    }
    owgpio=(OneWireGPIO *)deviceManager->getDevice("3af51321000000cb1");
    temppid=pids->getPID("Fermenter2");
     owgpio->setName("GlycolFermenter2");
     if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed

         Connection *conn=new Connection((Device *)owgpio,temppid,COOLING);
         connections->addConnection(conn);
     }
    owgpio=(OneWireGPIO *)deviceManager->getDevice("3a2d0e210000003e0");
    owgpio->setName("GlycolPump");
    if (owgpio!=NULL) { //if this didnt work, get device or get pid failed
        Connection *conn=new Connection((Device *)owgpio,"[CFermenter1 | [CFermenter2");
    	connections->addConnection(conn);
    }

    lastTime=millis();


}

void loop(){
	unsigned long time=millis();
	bLink->receive();
	//delay(50); //slow things down a bit
	if((time-lastTime)>2000) { //update PIDs once every 2 seconds
		uint32_t freemem = System.freeMemory();
		bLink->printDebug("free memory: %d Bytes",freemem);
		bLink->printDebug("update PIDs");
		pids->updatePIDs();
		//delay(2000);
		bLink->printDebug("update connections");
		connections->update(); //update outputs
		lastTime=time;
		//delay(2000);
		bLink->printDebug("looping");
	}
}
