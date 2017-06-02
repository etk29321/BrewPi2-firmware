#include "Brewpi.h"
SYSTEM_THREAD(ENABLED);
//SYSTEM_MODE(SEMI_AUTOMATIC);

//SerialDebugOutput debugOutput(38400, ALL_LEVEL); // use a faster baudrate and log only warnings or more severe

#define FERM1SET 50
#define FERM2SET 50
#define GLYCOLSET 44

BrewLink *bLink;
DeviceManager *deviceManager;
PIDs *pids;
Connections *connections;
Storage *storage;

String hostname=String("brewpi.bakersapex.com");
IPAddress syslogServer(192,168,8,20);
Syslog syslog(hostname,syslogServer,514);

unsigned long lastTime;
unsigned long lastReportTime;
unsigned long lastConnectAttempt;
bool wifiFlap;

void setup()
{
	//we want manual wifi and no particle cloud
	//WiFi.off();
	SYSTEM_THREAD(ENABLED);
	SYSTEM_MODE(MANUAL);
    //load WIFI
	delay(1000); // wait a sec to get serial term up and ready
	wifiFlap=false;
	lastConnectAttempt=millis();
	lastReportTime=millis();
    bLink=new BrewLink();
	//bLink->begin(); //start wifi server
    IPAddress myAddress(192,168,8,147);
    IPAddress netmask(255,255,255,0);
    IPAddress gateway(192,168,8,1);
    IPAddress dns(192,168,8,1);
    WiFi.setStaticIP(myAddress, netmask, gateway, dns);

      // now let's use the configured IP
    WiFi.useStaticIP();

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
    storage=new Storage();
    char * reply=deviceManager->deviceSearch();  //autodiscover any attached devices
    free(reply);

    // dynamic settings
    reply=storage->read();
    free(reply);

    // hardcoded settings
    /*
    uint8_t ferm1set;
    uint8_t ferm2set;
    uint8_t glycolset;
    ferm1set=EEPROM.read(0);
    ferm2set=EEPROM.read(1);
    glycolset=EEPROM.read(2);
    if (ferm1set==0) {
    	ferm1set=FERM1SET;
    }
    if (ferm2set==0) {
    	ferm2set=FERM2SET;
    }
    if (glycolset==0) {
    	glycolset=GLYCOLSET;
    }
	//  Add some connections
    OneWireTempSensor *temp=(OneWireTempSensor *)deviceManager->getDevice("28fae0a006000001");
	if(temp!=NULL) {temp->setName("Fermenter1");}
	if(temp!=NULL) {temp->setStrName(String("Fermenter1"));}

    temp=(OneWireTempSensor *)deviceManager->getDevice("28e146a106000082");
    if(temp!=NULL) {temp->setName("Glycol");}
	if(temp!=NULL) {temp->setStrName(String("Glycol"));}

	temp=(OneWireTempSensor *)deviceManager->getDevice("287ea2a20600005a");
	if(temp!=NULL) {temp->setName("Fermenter2");}
	if(temp!=NULL) {temp->setStrName(String("Fermenter2"));}

    PID *temppid=pids->getPID("Fermenter1");
    OneWireGPIO *owgpio=(OneWireGPIO *)deviceManager->getDevice("3af51321000000cb0");
    if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
        temppid->setSetPoint((double)ferm1set);
    	owgpio->setName("GlycolFermenter1");
    	owgpio->setStrName(String("GlycolFermenter1"));
        Connection *conn=new Connection((Device *)owgpio,temppid,COOLING);
        connections->addConnection(conn);
    }
    owgpio=(OneWireGPIO *)deviceManager->getDevice("3af51321000000cb1");
    if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
    	  owgpio->setName("HeatingFermenter1");
      	owgpio->setStrName(String("HeatingFermenter1"));
          Connection *conn=new Connection((Device *)owgpio,temppid,HEATING);
          connections->addConnection(conn);
    }

    owgpio=(OneWireGPIO *)deviceManager->getDevice("3a1b20210000003b0");
    temppid=pids->getPID("Fermenter2");
     if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
    	 temppid->setSetPoint((double)ferm2set);
    	 owgpio->setName("GlycolFermenter2");
     	owgpio->setStrName(String("GlycolFermenter2"));
         Connection *conn=new Connection((Device *)owgpio,temppid,COOLING);
         connections->addConnection(conn);
     }
     owgpio=(OneWireGPIO *)deviceManager->getDevice("3a1b20210000003b1");
     if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
     	   owgpio->setName("HeatingFermenter2");
       	owgpio->setStrName(String("HeatingFermenter2"));
           Connection *conn=new Connection((Device *)owgpio,temppid,HEATING);
           connections->addConnection(conn);
     }

    owgpio=(OneWireGPIO *)deviceManager->getDevice("3a2d0e210000003e0");
    if (owgpio!=NULL) { //if this didnt work, get device or get pid failed
        owgpio->setName("GlycolPump");
    	owgpio->setStrName(String("GlycolPump"));
    	Connection *conn=new Connection((Device *)owgpio,"[CFermenter1 | [CFermenter2");
    	connections->addConnection(conn);
    }

    // c{"Connections":[{"Add":{"OutputDevice":"3a2d0e210000003e1","mode":0.00,"PID":"287ea2a20600005a"}}]}
    owgpio=(OneWireGPIO *)deviceManager->getDevice("3a2d0e210000003e1");
    temppid=pids->getPID("Glycol");
    if (owgpio!=NULL && temppid!=NULL) { //if this didnt work, get device or get pid failed
        temppid->setSetPoint((double)glycolset);
        owgpio->setName("GlycolChiller");
    	owgpio->setStrName(String("GlycolChiller"));
    	Connection *conn=new Connection((Device *)owgpio,temppid,COOLING);
    	connections->addConnection(conn);
    }
     */
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
	if((time-lastReportTime)>60000) { //report debug info once every 60 seconds
		uint32_t freemem = System.freeMemory();
		bLink->printDebug("free memory: %d Bytes",freemem);
		/*if (freemem<20000) { // something is causing a memory leak that once started crashes us.  restart proactively for now.
			System.reset();
		}*/
		char *reply=bLink->cmdStatus();
		if (reply !=NULL){
			free(reply);
		}
		lastReportTime=time;
		//bLink->begin(); //restart wifi server

	}

	// WiFi restart event detection.  It'd be nice if photon threw an interrupt or event for this.....
	if (!WiFi.ready()) {
		wifiFlap=true;
	}
	if (wifiFlap && WiFi.ready()) {
		wifiFlap=false;
		bLink->stop();
		bLink->begin(); //restart wifi server
		bLink->printDebug("BrewPi just recovered from WiFi flap");

	}

}





