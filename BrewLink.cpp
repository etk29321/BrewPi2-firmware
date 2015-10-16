#include "BrewLink.h"
#include "Brewpi.h"


/***********************************************************************
**
** Note: Currently this object only allows a single TCP connection.  No
** new connections will be processed while that connection is active.
** This is due to the TCPServer code in wiring using a static TCPClient
** object.  This means that TCPServer does not support multiple concurrent
** connections->
**
***********************************************************************/


/***********************************************************************
**
** Function: BrewLink
** Purpose: BrewLink object constructor
**
***********************************************************************/
BrewLink::BrewLink(){
	Serial.begin(9600);
	wifiserver = new TCPServer(8080);
	clients=NULL;
	serialBufPos=0;
	singleClientBufPos=0;
}

/***********************************************************************
**
** Function: ~BrewLink
** Purpose: BrewLink object destructor
**
***********************************************************************/
BrewLink::~BrewLink(){
	if (wifiserver!=NULL)
		delete wifiserver;
	Serial.end();
	BrewLinkClient *currclient=clients;
	BrewLinkClient *prevclient;
	while (currclient!=NULL) { // clean up any still attached client structs
		currclient->client->flush();
		currclient->client->stop();  //gracefully disconnect
		prevclient=currclient;
		currclient=currclient->next;
		free(prevclient);
	}
}

/***********************************************************************
**
** Function: BrewLink::begin
** Purpose: Start BrewLink TCP server
**
***********************************************************************/
void BrewLink::begin(){
	wifiserver->begin();
}


/***********************************************************************
**
** Function: BrewLink::receive
** Purpose: checks for waiting input from any client that needs to
** be processed.
**
***********************************************************************/
void BrewLink::receive(){
	//first check the serial connection for input
	//printDebug("recieve started");
	char *replybuf=NULL;
	while (Serial.available()) {
		char inByte = Serial.read();
		Serial.write(inByte); //echo on
		if (inByte=='\n' || inByte=='\r') {  //Commands terminate with a newline or carrage return
			serialCmdBuf[serialBufPos]='\0'; //terminate C string
			replybuf=processCmd(serialCmdBuf);
			if (replybuf!=NULL) {
				Serial.println(replybuf);
				free(replybuf);
				replybuf=NULL;
			}
			serialBufPos=0;
			while (inByte=='\n'|| inByte=='\r') { //flush the buffer to the start of the next command
				inByte = Serial.read();
			}
		} else {
			serialCmdBuf[serialBufPos]=inByte;
			serialBufPos++;
		}
	}


	/* If TCPServer supported concurent connections we'd uncomment this
	//now loop though all connected TCP clients
	BrewLinkClient *currclient=clients;
	while (currclient!=NULL) {
		//printDebug("Checking for data from client %x",currclient);
		checkClient(currclient);
		currclient=currclient->next;
	}
	//check for new connections
	TCPClient newcl=wifiserver->available();  //TCPServer.available always returns an object, so we have to grab it and check if its a valid connection
	if (newcl.connected()) {
		printDebug("TCP Connection recieved from %x.", (int) &newcl);
		BrewLinkClient *newclient=(BrewLinkClient *)malloc(sizeof(BrewLinkClient));
		if (newclient!=NULL) { // ensure successful malloc
			newclient->lastHeard=millis();
			newclient->client=newcl;
			newclient->next=clients;
			clients=newclient;
		}
	}
	*/


	if (singleClient.connected()) {
		int bytesRead=0;
		while (singleClient.available()) {
			char inByte = singleClient.read();
			//printDebug("read %c",inByte);
			if (inByte=='\n'|| inByte=='\r') {  //Commands terminate with a newline
				singleClientCmdBuf[singleClientBufPos]='\0'; //terminate C string
				replybuf=processCmd(singleClientCmdBuf);
				if (replybuf!=NULL) {
					singleClient.println(replybuf);
					free(replybuf);
					replybuf=NULL;
				}
				singleClientBufPos=0;
				while (inByte=='\n'|| inByte=='\r') { //flush the buffer to the start of the next command
					inByte = singleClient.read();
				}
			} else {
				singleClientCmdBuf[singleClientBufPos]=inByte;
				singleClientBufPos++;
			}
			bytesRead++;
			lastHeard=millis();
		}
		if (bytesRead==0) { // handle timeouts if we recieved nothing from this client
			unsigned int time = millis();
			if (lastHeard>time) {  //millis overflowed so we'll give the client a pass this time
				lastHeard=millis();
			} else {
				unsigned int deltaTime = time - lastHeard;
				if (deltaTime>TCPTIMEOUT) {
					printDebug("Disconnecting client. Exceeded timeout of %d milliseconds", TCPTIMEOUT);
					singleClient.flush();
					singleClient.stop();
				}
			}
		}
	}  else {
		//check for new connections
		//printDebug("No client connected. Checking for new clients.");
		singleClient=wifiserver->available();  //TCPServer.available always returns an object, so we have to grab it and check if its a valid connection
		if (singleClient.connected()) {  //reset timeout on new connection
			lastHeard=millis();
		}
	}





	//printDebug("recieve completed");

}

/***********************************************************************
**
** Function: BrewLink::checkClient
** Purpose: processes a single client, reading all avaiable data and
** enforcing timeouts.
**
***********************************************************************/
void BrewLink::checkClient(BrewLinkClient *client) {
	if (client->client->connected()) {
		int bytesRead=0;
		while (client->client->available()) {
			char inByte = client->client->read();
			printDebug("Recieved TCP byte %c from client %x",inByte, (int)client->client);
			client->client->write(inByte); //echo back input to output for testing
			//printDebug("Wrote TCP byte %c to client %x",inByte, (int)client->client);
			bytesRead++;
			client->lastHeard=millis();
		}
		if (bytesRead==0) { // handle timeouts if we recieved nothing from this client
			unsigned int time = millis();
			if (client->lastHeard>time) {  //millis overflowed so we'll give the client a pass this time
				client->lastHeard=millis();
			} else {
				unsigned int deltaTime = time - client->lastHeard;
				if (deltaTime>TCPTIMEOUT) {
					printDebug("Disconnecting client %x. Exceeded timeout of %d seconds",(int)client->client, TCPTIMEOUT);
					disconnect(client);
				}
			}
		}
	} else {
		disconnect(client);
	}
	//printDebug("checkClient %x completed",(int)client);

}

/***********************************************************************
**
** Function: BrewLink::disconnect
** Purpose: disconnects a TCPClient and deletes its BrewLinkClient
** struct.
**
***********************************************************************/
void BrewLink::disconnect(BrewLinkClient *client) {
	client->client->flush();
	client->client->stop();  //gracefully disconnect
	if (client->prev->next!=NULL) {
		client->prev->next=client->next; // remove this client from the list
	} else { //we're the head of the list
		clients=client->next;
	}
	free(client);
}

/***********************************************************************
**
** Function: BrewLink::printDebug
** Purpose: prints debug messages to serial.  We wrap this call to make
** future modifications to how we print debug messages easier.
**
***********************************************************************/
void BrewLink::printDebug(const char *format, ...) {
	char buf[2048];
	va_list args;
	va_start(args,format);
	int sizeneeded=vsnprintf(buf,2048,format,args);
	Serial.println(buf);
	Serial.flush(); //wait for buffers to flush.  We want to make sure debug gets printed before a possible crash
	if (sizeneeded>2047){
		printDebug("Error: Serial Buffer Overflow.  Buffer is %d, String was %d characters",2048,sizeneeded);
	}
	va_end(args);
}

/***********************************************************************
**
** Function: BrewLink::processCmd
** Purpose: parse and act on incoming command strings
**
***********************************************************************/
char *BrewLink::processCmd(char *cmd) {
	char *reply;
	printDebug("Recieved command: %s",cmd);
	IPAddress localIP;
	byte mac[6];
	JSONObj *json;
	//JSONElement *jsonel;
	switch (*cmd) {
	case 'i':
		localIP = WiFi.localIP();
		WiFi.macAddress(mac);
		reply=(char *)malloc(sizeof(char)*(strlen(cmd)+45));
//			           0         1         2         3         4
//			           01234567890123456789012345678901234567890
//		               IP:255.255.255.255 MAC: ff:ff:ff:ff:ff:ff
		sprintf(reply,"IP:%d.%d.%d.%d MAC: %0x:%0x:%0x:%0x:%0x:%0x",localIP[0],localIP[1],localIP[2],localIP[3],mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		break;
	case 'd':  //perform device search
		reply=cmdDeviceSearch();
		break;
	case 'p': // list all PIDs
		json=pids->jsonify();
		reply=json->jstringify();
		printDebug("reply: %s",reply);
		delete json;
		break;
	case 'c': // list all connections
		json=connections->jsonify();
		reply=json->jstringify();
		printDebug("reply: %s",reply);
		delete json;
		break;
	case 'l': //status command (LCD command)
		reply=cmdStatus();
		break;
	case 's': //set PID set point
		reply=cmdSet(cmd);
		break;
	case 't': // toggle GPIO
		reply=cmdToggle(cmd);
		break;
	default:
		reply=(char *)malloc(sizeof(char)*(strlen(cmd)+20));
//			           0         1         2         3         4
//			           01234567890123456789012345678901234567890
		sprintf(reply,"Unknown Cmd: %s",cmd);
	}
	return reply;
}

char *BrewLink::cmdDeviceSearch(){
	char *reply=deviceManager->deviceSearch();
	if (reply==NULL) {
		reply=(char *)malloc(sizeof(char)*40);
//			               0         1         2         3         4
//			               01234567890123456789012345678901234567890
		sprintf(reply,"Device manager returned no results");

	}
	return reply;
}

char *BrewLink::cmdStatus(){
	JSONObj *json=deviceManager->status();
	char *reply=json->jstringify();
	free(json);
	return reply;
}

char *BrewLink::cmdToggle(char *cmd){
	char * reply;
	char *jsontxt=cmd;
	Device *gpio=NULL;

	jsontxt++;
	JSONObj *json=new JSONObj(jsontxt);
	if (json!=NULL) {
		JSONElement *jsonel=json->getElement("dev");
		//printDebug("Getting device %s",jsonel->getValue());
		gpio=deviceManager->getDevice(jsonel->getValue());
		if (gpio!=NULL) {
			if(gpio->getValue()==LOW) {
				gpio->setOutput(HIGH);
				reply=(char *)malloc(sizeof(char)*30);
				sprintf(reply,"Set %s HIGH",jsonel->getValue());
			} else {
				gpio->setOutput(LOW);
				reply=(char *)malloc(sizeof(char)*30);
				//			   0         1         2         3         4
				//			   01234567890123456789012345678901234567890
				//             Set 3af51321000000cb1 HI/?
				sprintf(reply,"Set %s LOW",jsonel->getValue());				}
		} else {
			reply=(char *)malloc(sizeof(char)*(strlen(cmd)+20));
	//			           0         1         2         3         4
	//			           01234567890123456789012345678901234567890
			sprintf(reply,"Unknown Device: %s",cmd);
			//printDebug("reply: %s",reply);
		}
	} else {
		reply=(char *)malloc(sizeof(char)*(strlen(cmd)+30));
//			           0         1         2         3         4
//			           01234567890123456789012345678901234567890
		sprintf(reply,"Unable to parse command: %s",cmd);
		//printDebug("reply: %s",reply);
	}
	delete json;
	return reply;
}

char *BrewLink::cmdSet(char *cmd){
	char * reply;
	char *jsontxt=cmd;
	PID *temppid=NULL;

	jsontxt++;
	JSONObj *json=new JSONObj(jsontxt);
	if (json!=NULL) {
		//printDebug("parsed JSON");
		//delay(1000);
		JSONElement *jsonel=json->getFirstElement();
		if (jsonel!=NULL) {
			//printDebug("got first element 0x%x", (int) jsonel);
			//delay(1000);
			temppid=pids->getPID(jsonel->getName());
			//printDebug("got pid 0x%x", (int) temppid);
			//delay(1000);
			if (temppid!=NULL) {
				temppid->setSetPoint(strtod(jsonel->getValue(),NULL));
				//printDebug("setSetpoint");
				//delay(1000);
				reply=(char *)malloc(sizeof(char)*(15+strlen(jsonel->getName())+strlen(jsonel->getValue())));
					//			   0         1         2         3         4
					//			   01234567890123456789012345678901234567890
					sprintf(reply,"Set PID %s=%s",jsonel->getName(),jsonel->getValue());
			} else {
				reply=(char *)malloc(sizeof(char)*(strlen(cmd)+20));
	//			           0         1         2         3         4
	//			           01234567890123456789012345678901234567890
				sprintf(reply,"Unknown PID: %s",cmd);
				//printDebug("reply: %s",reply);
			}
		} else {
			reply=(char *)malloc(sizeof(char)*(strlen(cmd)+30));
	//			           0         1         2         3         4
	//			           01234567890123456789012345678901234567890
			sprintf(reply,"Unable to parse command: %s",cmd);
			//printDebug("reply: %s",reply);
		}
	} else {
		reply=(char *)malloc(sizeof(char)*(strlen(cmd)+30));
//			           0         1         2         3         4
//			           01234567890123456789012345678901234567890
		sprintf(reply,"Unable to parse command: %s",cmd);
		//printDebug("reply: %s",reply);
	}
	delete json;
	return reply;
}
