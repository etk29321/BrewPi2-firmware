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
void Storage::read() {
	fstable fs;
	pos=0;

	//read fstable
	EEPROM.get(pos,fs);


	//read devices
	pos+=sizeof(fstable);
	if (fs.devCount==0 || fs.devCount==255) {
		return; //invalid data in EEPROM
	}
	DEVSTORObj *devstore=new DEVSTORObj(fs.devCount);
	for (int i=0;i<fs.devCount;i++) {
		deventity dev;
		EEPROM.get(pos,dev);
		memcpy(devstore->devs[i],&dev,sizeof(deventity));
		pos+=sizeof(deventity);

	}

	//read pids
	if (fs.pidCount==0 || fs.pidCount==255) {
		return; //invalid data in EEPROM
	}
	PIDSTORObj *pidstore=new PIDSTORObj(fs.pidCount);
	for (int i=0;i<fs.pidCount;i++) {
		pidentity pid;
		EEPROM.get(pos,pid);
		memcpy(pidstore->pids[i],&pid,sizeof(pidentity));
		pos+=sizeof(pidentity);

	}

	//read connections
	if (fs.connCount==0 || fs.connCount==255) {
		return; //invalid data in EEPROM
	}
	CONNSTORObj *connstore=new CONNSTORObj(fs.connCount);
	for (int i=0;i<fs.connCount;i++) {
		connentity conn;
		EEPROM.get(pos,conn);
		memcpy(connstore->conns[i],&conn,sizeof(connentity));
		pos+=sizeof(connentity);

	}
	strPos=sizeof(fstable) + sizeof(deventity)*fs.devCount + sizeof(pidentity)*fs.pidCount + sizeof(connentity)*fs.connCount+1;

	apply(devstore,pidstore,connstore);

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
	    			dev->setCorF(devstore->devs[i]->CorF);
	    		}
	            break;
	        case DEVICE_HARDWARE_PIN:
	    		dev=deviceManager->getHWGPIODevice(devstore->devs[i]->pinpio);
	    		if (dev!=NULL) {
	    			dev->setGPIOMode(devstore->devs[i]->gpioMode);
	    		}
	            break;
	        case DEVICE_HARDWARE_ONEWIRE_2413:
	    		dev=deviceManager->getDevice(devstore->devs[i]->address,devstore->devs[i]->pinpio);
	    		if (dev!=NULL) {
	    			dev->setGPIOMode(devstore->devs[i]->gpioMode);
	    		}
	        	break;
	        default:
	        	dev=NULL;
		}
		if (dev!=NULL) {
			devstore->devs[i]->DeviceID=dev->DeviceID; //fix up DeviceID to make sure they match the deviceManager ID
			dev->setName(devstore->devs[i]->Name);
			dev->setStrName(String(devstore->devs[i]->Name));
		}
	}


	//pids
	for (int i=0;i<pidstore->pidCount;i++) {
		Device *dev=deviceManager->getDevice(pidstor->pids[i]->DeviceID);
		if (dev!=NULL) {
			PID *pid=pids->getPID(dev->getName());
			if(pid!=NULL) {
				pid=>setP(pidstore->pids[i]->p);
				pid=>setI(pidstore->pids[i]->i);
				pid=>setD(pidstore->pids[i]->d);
				pid=>setSetPoint(pidstore->pids[i]->setPoint);
				pid=>setMinStateTime(pidstore->pids[i]->minStateTimeSecs);
				pid=>setDeadBand(pidstore->pids[i]->deadBand);
				pid=>setPWMScale(pidstore->pids[i]->PWMScale);
			}
		}
	}

	//connections
	for (int i=0;i<connstore->connCount;i++) {
		switch(connstore->conns[i]->mode) {
			case PIDCooling:
			case PIDHeating:
				Device *dev=deviceManager->getDevice(connstore->conns[i]->outdevID);
				Device *piddev=deviceManager->getDevice(connstore->conns[i]->PIDdevID);
				if (dev!=NULL && piddev!=NULL) {
					PID *pid=pids->getPID(piddev->getName());
					if (pid!=NULL) {
						Connection *conn=new Connection(dev,pid,connstore->conns[i]->Pstate);
						connections->addConnection(conn);
					}
				}
				break;
			case DevMode:
				Device *outdev=deviceManager->getDevice(connstore->conns[i]->outdevID);
				Device *indev=deviceManager->getDevice(connstore->conns[i]->indevID);
				if (outdev!=NULL && indev!=NULL) {
					Connection *conn=new Connection(outdev,indev);
					connections->addConnection(conn);
				}
				break;
			case Custom:
				connent->outdevID=(conn->getOutDev())->DeviceID;
				exp=conn->getExp();

				Device *outdev=deviceManager->getDevice(connstore->conns[i]->outdevID);
				char *exp=readString(connstore->conns[i]->exp, connstore->conns[i]->explen);
				if (outdev!=NULL && exp!=NULL) {
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

void Storage::write() {
	DEVSTORObj *devstor=deviceManager->storeify();
	PIDSTORObj *pidstor=pids->storeify();
	fstable fs;
	fs.devCount=devstor->devCount;
	fs.pidCount=pidstor->pidCount;
	fs.connCount=conns->getNumConns();
	strPos=sizeof(fstable) + sizeof(deventity)*fs.devCount + sizeof(pidentity)*fs.pidCount + sizeof(connentity)*fs.connCount+1;
	pos=0;
	//write fstable
	EEPROM.put(pos,fs);
	pos+=sizeof(fstable);
	//write devices
	for (int i=0;i<devstor->devCount;i++) {
		EEPROM.put(pos,*(devstor->devs[i]));
		pos+=sizeof(deventity);
	}
	//write pids
	for (int i=0;i<pidstor->pidCount;i++) {
		EEPROM.put(pos,*(pidstor->pids[i]));
		pos+=sizeof(pidentity);
	}
	//write connections
	for (int i=0;i<conns->getNumConns();i++) {
		connentity connent;
		connent->outdevID=0;
		connent->PIDdevID=0;
		connent->Pstate=0;
		connent->indevID=0;
		connent->exp=EEPROM.length; //address of expression string
		connent->expLen=0; //length

		char *exp;
		Connection *conn=conns->getConnection(i);
		if (conn!=NULL){
			connent.mode.conn->getMode();
			switch(connent.mode) {
				case PIDCooling:
				case PIDHeating:
					connent->outdevID=(conn->getOutDev())->DeviceID;
					connent->PIDdevID=(conn->getInPID())->PIDID;
					connent->Pstate=conn->getPIDState();
					break;
				case DevMode:
					connent->outdevID=(conn->getOutDev())->DeviceID;
					connent->indevID=(conn->getInDev())->DeviceID;
					break;
				case Custom:
					connent->outdevID=(conn->getOutDev())->DeviceID;
					exp=conn->getExp();
					connent->exp=strPos; //address of expression string
					connent->expLen=strlen(exp); //length
					EEPROM.put(pos,*(conmstor->conns[i]));
					strPos=writeString(exp, strPos);
					break;
			}
			pos+=sizeof(connentity);
		}

	}
	delete devstor;
	delete pidstor;

}

int Storage::writeString(char *str, int pos) {
	if (str==NULL) {
		return pos++;
	}
	if (pos>EEPROM.length) {
		return pos;
	}
	int len=strlen(str);
	for(int i=0;i<=len;i++) {
		EEPROM.write(pos,str[i]);
		pos++;
		if (pos>EERPOM.length) {
			return pos;
		}
	}
	return pos;
}

char *Storage::readString(int pos, int len) {
	char *str=malloc(sizeof(char)*len);
	for(int i=0;i<len;i++) {
		str[i]=(char)EEPROM.read(pos);
		pos++;
		if (pos>EERPOM.length) {
			str[i]='\0';
			return str;
		}
	}
	str[i]='\0';  //ensure we always return a null terminated string.
	return str;
}
