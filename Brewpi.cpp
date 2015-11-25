#include "Brewpi.h"

//SerialDebugOutput debugOutput(38400, ALL_LEVEL); // use a faster baudrate and log only warnings or more severe


BrewLink *bLink;
DeviceManager *deviceManager;
PIDs *pids;
Connections *connections;

String hostname=String("brewpi.bakersapex.com");
IPAddress syslogServer(192,168,8,20);
Syslog syslog(hostname,syslogServer,514);

unsigned long lastTime;
unsigned long lastConnectAttempt;


void setup()
{
	//we want manual wifi and no particle cloud
	//SYSTEM_MODE(MANUAL);
	//SYSTEM_MODE(SEMI_AUTOMATIC);
	SYSTEM_THREAD(ENABLED);
    //load WIFI
	delay(1000); // wait a sec to get serial term up and ready

	lastConnectAttempt=millis();
    bLink=new BrewLink();
	bLink->begin(); //start wifi server
    WiFi.on();
 	WiFi.connect();
 	while (millis()-lastConnectAttempt < 30000 && !WiFi.ready()) {
 		delay(100);
 	}
	bLink->begin(); //start wifi server
    bLink->printDebug("Welcome to BrewPi!  System booting...");
 	pids=new PIDs();
    connections=new Connections();
    deviceManager=new DeviceManager();
    char * reply=deviceManager->deviceSearch();  //autodiscover any attached devices
    free(reply);

	//  Add some connections
    OneWireTempSensor *temp=(OneWireTempSensor *)deviceManager->getDevice("28fae0a006000001");
	if(temp!=NULL) {temp->setName("Fermenter1");}
    temp=(OneWireTempSensor *)deviceManager->getDevice("28e146a106000082");
    if(temp!=NULL) {temp->setName("Fermenter2");}
	temp=(OneWireTempSensor *)deviceManager->getDevice("287ea2a20600005a");
	if(temp!=NULL) {temp->setName("Glycol");}
    PID *temppid=pids->getPID("Fermenter1");
    OneWireGPIO *owgpio=(OneWireGPIO *)deviceManager->getDevice("3af51321000000cb0");
    if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
        temppid->setSetPoint(68);
    	owgpio->setName("GlycolFermenter1");
        Connection *conn=new Connection((Device *)owgpio,temppid,COOLING);
        connections->addConnection(conn);
    }
    owgpio=(OneWireGPIO *)deviceManager->getDevice("3af51321000000cb1");
    if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
    	  owgpio->setName("HeatingFermenter1");
          Connection *conn=new Connection((Device *)owgpio,temppid,HEATING);
          connections->addConnection(conn);
    }

    owgpio=(OneWireGPIO *)deviceManager->getDevice("3a1b20210000003b0");
    temppid=pids->getPID("Fermenter2");
     if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
    	 temppid->setSetPoint(68);
    	 owgpio->setName("GlycolFermenter2");
         Connection *conn=new Connection((Device *)owgpio,temppid,COOLING);
         connections->addConnection(conn);
     }
     owgpio=(OneWireGPIO *)deviceManager->getDevice("3a1b20210000003b1");
     if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
     	   owgpio->setName("HeatingFermenter2");
           Connection *conn=new Connection((Device *)owgpio,temppid,HEATING);
           connections->addConnection(conn);
     }

    owgpio=(OneWireGPIO *)deviceManager->getDevice("3a2d0e210000003e0");
    if (owgpio!=NULL) { //if this didnt work, get device or get pid failed
        owgpio->setName("GlycolPump");
    	Connection *conn=new Connection((Device *)owgpio,"[CFermenter1 | [CFermenter2");
    	connections->addConnection(conn);
    }

    // c{"Connections":[{"Add":{"OutputDevice":"3a2d0e210000003e1","mode":0.00,"PID":"287ea2a20600005a"}}]}
    owgpio=(OneWireGPIO *)deviceManager->getDevice("3a2d0e210000003e1");
    temppid=pids->getPID("Glycol");
    if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
        temppid->setSetPoint(44);
        owgpio->setName("GlycolChiller");
    	Connection *conn=new Connection((Device *)owgpio,temppid,COOLING);
    	connections->addConnection(conn);
    }

    lastTime=millis();
    bLink->printDebug("Setup completed!");


}

void loop(){
	unsigned long time=millis();
	bLink->receive();

	if((time-lastTime)>2000) { //update PIDs once every 2 seconds
		//uint32_t freemem = System.freeMemory();
		//bLink->printDebug("free memory: %d Bytes",freemem);
		pids->updatePIDs();


		connections->update(); //update outputs
		lastTime=time;


	}
}





