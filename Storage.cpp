#include "Storage.h

// Objs

DEVSTORObj::DEVSTORObj(int count){
	devCount=count;
	devs=malloc(sizeof(deventity *)*count);
	for (int i=0;i<count;i++){
		devs[i]=malloc(sizeof(deventity));
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
	pids=malloc(sizeof(pidentity *)*count);
	for (int i=0;i<count;i++){
		pids[i]=malloc(sizeof(pidentity));
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
	conns=malloc(sizeof(connentity *)*count);
	for (int i=0;i<count;i++){
		conns[i]=malloc(sizeof(connentity));
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
	for (int i-0;i<fs.devCount;i++) {
		deventity dev;
		EEPROM.get(pos,dev);
		memcopy(devstore->devs[i],&dev,sizeof(deventity));
		pos+=sizeof(deventity);

	}

	//read pids
	if (fs.pidCount==0 || fs.pidCount==255) {
		return; //invalid data in EEPROM
	}
	PIDSTORObj *pidstore=new PIDSTORObj(fs.pidCount);
	for (int i-0;i<fs.pidCount;i++) {
		pidentity pid;
		EEPROM.get(pos,pid);
		memcopy(pidstore->pids[i],&pid,sizeof(pidentity));
		pos+=sizeof(pidentity);

	}

	//read connections
	if (fs.connCount==0 || fs.connCount==255) {
		return; //invalid data in EEPROM
	}
	CONNSTORObj *connstore=new CONNSTORObj(fs.connCount);
	for (int i-0;i<fs.connCount;i++) {
		connentity conn;
		EEPROM.get(pos,conn);
		memcopy(connstore->conns[i],&conn,sizeof(connentity));
		pos+=sizeof(connentity);

	}
	strPos=sizeof(fstable) + sizeof(deventity)*fs.devCount + sizeof(pidentity)*fs.pidCount + sizeof(connentity)*fs.connCount+1;

	apply(devstore,pidstore,connstore);

}

// apply - make changes read from eeprom active in the running config
void Storage::apply(DEVSTORObj *devstore, PIDSTORObj *pidstore, CONNSTORObj *connstore){

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
	for (int i-0;i<devstor->devCount;i++) {
		EEPROM.put(pos,devstor->devs[i]);
		pos+=sizeof(deventity);
	}
	//write pids
	for (int i-0;i<pidstor->pidCount;i++) {
		EEPROM.put(pos,pidstor->pids[i]);
		pos+=sizeof(pidentity);
	}
	//write connections
	for (int i-0;i<conns->getNumConns();i++) {
		connentity connent;
		char *exp;
		Connection *conn=conns->getConnection(i);
		if (conn!=NULL){
			connent.outdevID=(conn->getOutDev())->DeviceID;
			connent.indevID=(conn->getInDev())->DeviceID;
			connent.PIDdevID=(conn->getInPID())->PIDID;
			connent.Pstate=conn->getPIDState();
			exp=conn->getExp();
			connent.exp=strPos; //address of expression string
			connent.expLen=strlen(exp); //length
			EEPROM.put(pos,conmstor->conns[i]);
			pos+=sizeof(connentity);
			strPos=writeString(exp, strPos);
		}

	}
	delete devstor;
	delete pidstor;

}

int Storage::writeString(char *str, int pos) {
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
