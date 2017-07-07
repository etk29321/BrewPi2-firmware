#include "Storage.h"

// Objs

DEVSTORObj::DEVSTORObj(int count){
	devCount=count;
	devs=(deventity **)malloc(sizeof(deventity *)*count);
	for (int i=0;i<count;i++){
		devs[i]=(deventity *)malloc(sizeof(deventity));
	}
	return;
}

DEVSTORObj::~DEVSTORObj(){
	for (int i=0;i<devCount;i++){
		free(devs[i]);
	}
	free(devs);
	return;
}

PIDSTORObj::PIDSTORObj(int count){
	pidCount=count;
	pids=(pidentity **)malloc(sizeof(pidentity *)*count);
	for (int i=0;i<count;i++){
		pids[i]=(pidentity *)malloc(sizeof(pidentity));
	}
	return;
}

PIDSTORObj::~PIDSTORObj(){
	for (int i=0;i<pidCount;i++){
		free(pids[i]);
	}
	free(pids);
	return;
}

CONNSTORObj::CONNSTORObj(int count){
	connCount=count;
	conns=(connentity **)malloc(sizeof(connentity *)*count);
	for (int i=0;i<count;i++){
		conns[i]=(connentity *)malloc(sizeof(connentity));
	}
	return;
}

CONNSTORObj::~CONNSTORObj(){
	for (int i=0;i<connCount;i++){
		free(conns[i]);
	}
	free(conns);
	return;
}

Storage::Storage(){
	length = EEPROM.length();
	pos=0;
	strPos=0;
}

//read
char *Storage::read() {
	char *reply=NULL;
	fstable fs;
	pos=0;
	NetworkBlock netconfig;

	//read fstable
	EEPROM.get(pos,fs);


	pos+=sizeof(fstable);
	if (fs.devCount==0 || fs.devCount==255) {
		return NULL; //invalid data in EEPROM
	}

	//read network config
	EEPROM.get(pos,netconfig);
	pos+=sizeof(NetworkBlock);
	piNet.setConfig(&netconfig);


	//read devices
	DEVSTORObj *devstore=new DEVSTORObj(fs.devCount);
	for (int i=0;i<fs.devCount;i++) {
		deventity dev;
		EEPROM.get(pos,dev);
		memcpy(devstore->devs[i],&dev,sizeof(deventity));
    	bLink->printDebug("Reading device:%x (%s) stating at %d",devstore->devs[i]->DeviceID,devstore->devs[i]->Name,pos);
		pos+=sizeof(deventity);

	}

	//read pids
	if (fs.pidCount==0 || fs.pidCount==255) {
		return NULL; //invalid data in EEPROM
	}
	PIDSTORObj *pidstore=new PIDSTORObj(fs.pidCount);
	for (int i=0;i<fs.pidCount;i++) {
		pidentity pid;
		EEPROM.get(pos,pid);
		memcpy(pidstore->pids[i],&pid,sizeof(pidentity));
    	bLink->printDebug("Reading pident:%x stating at %d",pidstore->pids[i]->DeviceID,pos);
		pos+=sizeof(pidentity);

	}

	//read connections
	if (fs.connCount==0 || fs.connCount==255) {
		return NULL; //invalid data in EEPROM
	}
	CONNSTORObj *connstore=new CONNSTORObj(fs.connCount);
	for (int i=0;i<fs.connCount;i++) {
		connentity conn;
		EEPROM.get(pos,conn);
		memcpy(connstore->conns[i],&conn,sizeof(connentity));
    	bLink->printDebug("Writing connent:%x stating at %d",connstore->conns[i]->outdevID,pos);
		pos+=sizeof(connentity);

	}
	strPos=sizeof(fstable) + sizeof(deventity)*fs.devCount + sizeof(pidentity)*fs.pidCount + sizeof(connentity)*fs.connCount+1;

	apply(devstore,pidstore,connstore);
	reply=(char *)malloc(sizeof(char)*(15));
	//			   0         1         2         3         4
	//			   01234567890123456789012345678901234567890
	sprintf(reply,"Config Loaded!");
	return reply;
}

// apply - make changes read from eeprom active in the running config
void Storage::apply(DEVSTORObj *devstore, PIDSTORObj *pidstore, CONNSTORObj *connstore){
	//devices
	for (int i=0;i<devstore->devCount;i++) {
		Device *dev;

		switch (devstore->devs[i]->DeviceHardware) {
	    	case DEVICE_HARDWARE_ONEWIRE_TEMP:
	    		dev=deviceManager->getDevice(devstore->devs[i]->address);

	    		if (dev!=NULL) {
			    	bLink->printDebug("Updating temp sensor device:%x (%s)",i,dev->getName());

	    			((OneWireTempSensor *)dev)->setCorF((TempFormat)(devstore->devs[i]->CorF));
	    		}
	            break;
	        case DEVICE_HARDWARE_PIN:
	    		dev=deviceManager->getHWGPIODevice(devstore->devs[i]->pinpio);
	    		if (dev!=NULL) {
			    	bLink->printDebug("Updating HE GPIO device:%x (%s)",i,dev->getName());

	    			((HardwareGPIO *)dev)->setGPIOMode((PinMode)(devstore->devs[i]->gpioMode));
	    		}
	            break;
	        case DEVICE_HARDWARE_ONEWIRE_2413:
	    		dev=deviceManager->getDevice(devstore->devs[i]->address,devstore->devs[i]->pinpio);
	    		if (dev!=NULL) {
			    	bLink->printDebug("Updating OneWire GPIO device:%x (%s)",i,dev->getName());

	    			((OneWireGPIO *)dev)->setGPIOMode((PinMode)(devstore->devs[i]->gpioMode));
	    		}
	        	break;
	        default:
	        	dev=NULL;
		}
		if (dev!=NULL) {
			//devstore->devs[i]->DeviceID=dev->DeviceID; //fix up DeviceID to make sure they match the deviceManager ID
			dev->DeviceID=devstore->devs[i]->DeviceID; //fix up DeviceID to make sure they match the deviceManager ID
			dev->setName(devstore->devs[i]->Name);
			dev->setStrName(String(devstore->devs[i]->Name));
		}
	}


	//pids
	for (int i=0;i<pidstore->pidCount;i++) {
		Device *dev=deviceManager->getDevice(pidstore->pids[i]->DeviceID); //get device by address. PIDs only attach to temp sensors
		if (dev!=NULL) {
			PID *pid=pids->getPID(dev->getName());
			if(pid!=NULL) {
		    	bLink->printDebug("Updating PID:%x (%s)",i,pid->getName());
				pid->setP(pidstore->pids[i]->p);
				pid->setI(pidstore->pids[i]->i);
				pid->setD(pidstore->pids[i]->d);
				pid->setSetPoint(pidstore->pids[i]->setPoint);
				pid->setMinStateTime(pidstore->pids[i]->minStateTimeSecs);
				pid->setDeadBand(pidstore->pids[i]->deadBand);
				pid->setPWMScale(pidstore->pids[i]->PWMScale);
			}
		}
	}

	//connections
	for (int i=0;i<connstore->connCount;i++) {
		Device *dev;
		Device *piddev;
		Device *outdev;
		Device *indev;
		char *exp;
		switch(connstore->conns[i]->mode) {
			case PIDCooling:
			case PIDHeating:
				dev=deviceManager->getDevice(connstore->conns[i]->outdevID);
				piddev=deviceManager->getDevice(connstore->conns[i]->inPIDdevID);
				if (dev!=NULL && piddev!=NULL) {
			    	bLink->printDebug("Creating PID Connection:%x (%s)",i,dev->getName());
					PID *pid=pids->getPID(piddev->getName());
					if (pid!=NULL) {
						Connection *conn=new Connection(dev,pid,(PIDState)connstore->conns[i]->Pstate);
						connections->addConnection(conn);
					}
				}
				break;
			case DevMode:
				outdev=deviceManager->getDevice(connstore->conns[i]->outdevID);
				indev=deviceManager->getDevice(connstore->conns[i]->indevID);
				if (outdev!=NULL && indev!=NULL) {
			    	bLink->printDebug("Creating Dev Connection:%x (%s)",i,dev->getName());
					Connection *conn=new Connection(outdev,indev);
					connections->addConnection(conn);
				}
				break;
			case Custom:
				outdev=deviceManager->getDevice(connstore->conns[i]->outdevID);
				exp=readString(connstore->conns[i]->exp, connstore->conns[i]->explen);
				if (outdev!=NULL && exp!=NULL) {
			    	bLink->printDebug("Creating Custom Connection:%x (%s)",i,dev->getName());
					Connection *conn=new Connection(outdev,exp);
					connections->addConnection(conn);
					free(exp);
				}

				break;
		}
	}
	delete devstore;
	delete pidstore;
	delete connstore;

}


//write

char *Storage::write() {
	char *reply=NULL;
	DEVSTORObj *devstor=deviceManager->storeify();
	PIDSTORObj *pidstor=pids->storeify();
	NetworkBlock *netconfig=piNet.getConfig();
	fstable fs;
	fs.devCount=devstor->devCount;
	fs.pidCount=pidstor->pidCount;
	fs.connCount=connections->getNumConns();
	strPos=sizeof(fstable) + sizeof(NetworkBlock) + sizeof(deventity)*fs.devCount + sizeof(pidentity)*fs.pidCount + sizeof(connentity)*fs.connCount+1;
	pos=0;
	//write fstable
	bLink->printDebug("Writing fstable stating at %d",pos);

	EEPROM.put(pos,fs);
	pos+=sizeof(fstable);
	//write network config
	bLink->printDebug("Writing networkBlock stating at %d",pos);

	EEPROM.put(pos,*netconfig);
	pos+=sizeof(NetworkBlock);
	free(netconfig);
	//write devices
	for (int i=0;i<devstor->devCount;i++) {
		EEPROM.put(pos,*(devstor->devs[i]));
    	bLink->printDebug("Writing device:%x (%s) stating at %d",devstor->devs[i]->DeviceID,devstor->devs[i]->Name,pos);

		pos+=sizeof(deventity);

	}
	//write pids
	for (int i=0;i<pidstor->pidCount;i++) {
		EEPROM.put(pos,*(pidstor->pids[i]));
    	bLink->printDebug("Writing pident:%x stating at %d",pidstor->pids[i]->DeviceID,pos);

		pos+=sizeof(pidentity);
	}
	//write connections
	for (int i=0;i<connections->getNumConns();i++) {
		connentity connent;
		connent.outdevID=0xdb;
		connent.mode=0xdb;
		connent.indevID=0xdb;
		connent.inPIDdevID=0xdb;
		connent.Pstate=0xdb;
		connent.exp=(uint16_t)EEPROM.length(); //address of expression string
		connent.explen=0xdb; //length

		char *exp;
		Connection *conn=connections->getConnection(i);
		PID *inpid;
		Device *dev;
		if (conn!=NULL){
			connent.mode=conn->getMode();
			switch(connent.mode) {
				case PIDCooling:
				case PIDHeating:
					connent.outdevID=(conn->getOutDev())->DeviceID;
					inpid=conn->getInPID();
					if (inpid!=NULL) {
						dev=inpid->getDevice();
						if(dev!=NULL) {
							connent.inPIDdevID=dev->DeviceID;
						} else{
							connent.inPIDdevID=0;
						}
					}
					connent.Pstate=conn->getPIDState();
					break;
				case DevMode:
					connent.outdevID=(conn->getOutDev())->DeviceID;
					connent.indevID=(conn->getInDev())->DeviceID;
					break;
				case Custom:
					connent.outdevID=(conn->getOutDev())->DeviceID;
					exp=conn->getExp();
					connent.exp=strPos; //address of expression string
					connent.explen=strlen(exp); //length
					//EEPROM.put(pos,*(conmstor->conns[i]));
					strPos=writeString(exp, strPos);
					break;
			}
			EEPROM.put(pos,connent);
			JSONObj *json=conn->jsonify();
			char *str=json->jstringify();
	    	bLink->printDebug("Writing connent:i=%x devID=%x stating at %d -- %s",i,connent.outdevID,pos,str);
	    	delete json;
	    	free(str);
			pos+=sizeof(connentity);
		}

	}
	delete devstor;
	delete pidstor;
	reply=(char *)malloc(sizeof(char)*(15));
	//			   0         1         2         3         4
	//			   01234567890123456789012345678901234567890
	sprintf(reply,"Config Stored!");
	return reply;
}

int Storage::writeString(char *str, int pos) {
	if (str==NULL) {
		return pos++;
	}
	if (pos>EEPROM.length()) {
		return pos;
	}
	int len=strlen(str);
	for(int i=0;i<=len;i++) {
		EEPROM.write(pos,str[i]);
		pos++;
		if (pos>EEPROM.length()) {
			return pos;
		}
	}
	return pos;
}

char *Storage::readString(int pos, int len) {
	char *str=(char *)malloc(sizeof(char)*len);
	int i;
	for(i=0;i<len;i++) {
		str[i]=(char)EEPROM.read(pos);
		pos++;
		if (pos>EEPROM.length()) {
			str[i]='\0';
			return str;
		}
	}
	str[i]='\0';  //ensure we always return a null terminated string.
	return str;
}

void Storage::clear(){
	for(int i=0;i<EEPROM.length();i++) {
		EEPROM.write(i,0xFF);
	}
}

void Storage::dump(){
	char line[128];
	int l=0;
	uint8_t val;
	for(int i=0;i<EEPROM.length();i++) {
		val=EEPROM.read(i);
		char *str=&(line[l]);
		sprintf(str,"%0x ",val);
		bLink->printDebug("%d: %0x",i,val);

		l+=3;
		if(l>=128) {
			l=0;
			bLink->printDebug("%d: %s",i,line);
		}
	}
}
