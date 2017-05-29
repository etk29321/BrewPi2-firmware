#include "Storage.h

// Objs

DEVSTORObj::DEVSTORObj(int count){
	devCount=count;
	devs=malloc(sizeof(deventity)*count);
	return;
}

DEVSTORObj::~DEVSTORObj(){
	free(devs);
	return;
}

PIDSTORObj::PIDSTORObj(int count){
	pidCount=count;
	pids=malloc(sizeof(pidentity)*count);
	return;
}

PIDSTORObj::~PIDSTORObj(){
	free(pids);
	return;
}

CONNSTORObj::CONNSTORObj(int count){
	connCount=count;
	conns=malloc(sizeof(connentity)*count);
	return;
}

CONNSTORObj::~CONNSTORObj(){
	free(conns);
	return;
}

Storage::Storage(){
	length = EEPROM.length();
	pos=0;
}

//read


//write

void Storage::write() {

}
