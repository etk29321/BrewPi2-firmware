#include "PID.h"

PID::PID(Device *newdev, double P, double I, double D, double dBand, double PWMScalein){
	if (newdev!=NULL) {
		Kp=P;
		Ki=I;
		Kd=D;
        dev=newdev;
	} else {
		dev=NULL;
	}
	setPoint=80;
	prevError=0;
	prevTime=millis();
	deadBand=dBand;
    PWMScale=PWMScalein;
	I=0;
	temp=setPoint;
	state=IDLE;
	lastStateChange=0;
	minStateTime=120000; //2 minutes
}

PID::~PID() {

}

void PID::setSetPoint(double newtemp) {
	setPoint=newtemp;
}


void setP(double newP){
	P=newP;
}
void setI(double newI){
	I=newI;
}
void setD(double newD){
	D=newD;
}
void setMinStateTime(unsigned long newMinStateTime){
	minStateTime=newMinStateTime;
}
void setDeadBand(double newBand){
	deadBand=newBand;
}
void setPWMScale(double newScale){
	PWMScale=newScale;
}


char *PID::getName(){
	if(dev!=NULL) {
		return dev->getName();
	}
	return NULL;
}

String PID::getStrName(){
	return dev->getStrName();
}

void PID::update(){
	unsigned long stime=millis();
	if (dev==NULL) {
		bLink->printDebug("ERROR: PID::update called with NULL dev");
		return;
	}
	temp=dev->getValue();
#ifndef XCODE
    //bLink->printDebug("get Temp took %lu milliseconds.",millis()-stime);
#else
    printf("get Temp took %lu milliseconds.\n",millis()-stime);
#endif
	double P=setPoint-temp;
	unsigned long stateTime=stime-lastStateChange;
	if (state==COOLING && P>=0 && (stateTime>minStateTime || stateTime<0)) {
		state=IDLE;
		//String strName=getStrName();
		//String msg=String("PID" + strName+ " changed to IDLE.");
		//syslog.log(msg);
		char msgbuf[246];
        sprintf(msgbuf,"PID %s changed to IDLE",getName());
        syslog.log(msgbuf);
		lastStateChange=stime;
		PIDvalue=0;
		I=0;
	} else {
		if (state==HEATING && P<=0 && (stateTime>minStateTime || stateTime<0)) {
			state=IDLE;
			//String strName=getStrName();
			//String msg=String("PID" + strName+ " changed to IDLE.");
			//syslog.log(msg);
			char msgbuf[246];
	        sprintf(msgbuf,"PID %s changed to IDLE",getName());
	        syslog.log(msgbuf);
			lastStateChange=stime;
			PIDvalue=0;
			I=0;
		} else {
			if (state==IDLE && ((P>0 && P>deadBand) || (P<0 && (-P)>deadBand)) && (stateTime>minStateTime || stateTime<0)) {
				if(P>0){
					state=HEATING;
					//String strName=getStrName();
					//String msg=String("PID" + strName+ " changed to HEATING.");
					//syslog.log(msg);
					char msgbuf[246];
			        sprintf(msgbuf,"PID %s changed to HEATING",getName());
			        syslog.log(msgbuf);
					lastStateChange=stime;

				} else {
					state=COOLING;
					//String strName=getStrName();
					//String msg=String("PID" + strName+ " changed to COOLING.");
					//syslog.log(msg);
					char msgbuf[246];
			        sprintf(msgbuf,"PID %s changed to COOLING",getName());
			        syslog.log(msgbuf);
					lastStateChange=stime;

				}
			}
		}
	}
	if(state!=IDLE) {
		unsigned long dT;
		unsigned long curTime=millis();
		if (curTime>prevTime) {
			dT=(curTime - prevTime)/1000;
		} else {
			dT=0; //millis rolled over
		}
		I=I+P*dT;
		double D=(P-prevError)/dT;
		PIDvalue=Kp*P+Ki*I+Kd*D;
		prevError=P;
		prevTime=curTime;
	}
}

bool PID::heating(){
	if(state==HEATING) {
		return true;
	} else {
		return false;
	}
}

bool PID::cooling(){
	if(state==COOLING) {
		return true;
	} else {
		return false;
	}
}


uint8_t PID::PWMDutyCycle(){
	if((abs((int)PIDvalue)*PWMScale)>255) {
		return 255;
	}
	return (abs((int)PIDvalue)*PWMScale);
}

JSONObj *PID::jsonify(){
	JSONObj *json=new JSONObj();
	json->addElement("Sensor",dev->getName());
	json->addElement("Kp",Kp);
	json->addElement("Ki",Ki);
	json->addElement("Kd",Kd);
    json->addElement("setPoint",setPoint);
    json->addElement("temp",temp);
	json->addElement("PID",PIDvalue);
	json->addElement("state",state);
	json->addElement("lastStateChange",lastStateChange);
	json->addElement("minStateTime",minStateTime);
	json->addElement("stateTime",stateTime);
	json->addElement("heating",heating());
	json->addElement("cooling",cooling());
	json->addElement("deadBand",deadBand);
	json->addElement("PWMScale",PWMScale);
	json->addElement("PWMDutyCycle",PWMDutyCycle());
	return json;
}

void PID::storeify(pidentity *pident){
	if pident!=NULL{
		pident->DeviceID=dev->DeviceID; //deventity name
		pident->p=Kp;
		pident->i=Ki;
		pident->d=Kd;
		pident->setPoint=setPoint;
		pident->minStateTimeSecs=minStateTime; // in seconds
		pident->deadBand=deadBand;
		pident->PWMScale=PWMScale;
	}
}


/******************************************************************************
 *
 * PIDs
 *
 * ***************************************************************************/

PIDs::PIDs(){
	root=NULL;
	pidCount=0;
}

PIDs::~PIDs(){
	if(root!=NULL){
		for(int i=0;i<pidCount;i++) {
			delete root[i];
		}
		free(root);
	}
}

void PIDs::addPID(PID *p){
	if (root==NULL) {
		root=(PID **)malloc(sizeof(PID *));
		pidCount=1;
		root[0]=p;
	} else {
			PID **newroot=(PID **)realloc(root,sizeof(PID *)*(pidCount+1));
			if (newroot==NULL) {
				return;
			}
			if (newroot!=root) {
				root=newroot;
			}
			root[pidCount]=p;
			pidCount++;
	}
}

void PIDs::updatePIDs(){
	//unsigned long stime=millis();
	for(int i=0;i<pidCount;i++) {
		if (root[i]!=NULL) {
			root[i]->update();
			//bLink->printDebug("updated PID %s",root[i]->getName());
		}
	}
#ifndef XCODE
	//bLink->printDebug("updatePIDs took %lu milliseconds.",millis()-stime);
#else
    //printf("updatePIDs took %lu milliseconds.\n",millis()-stime);
#endif
}

PID *PIDs::getPID(char *name){ //PID name is the dev->getName() of the underlying device
	for(int i=0;i<pidCount;i++) {
		if (root[i]!=NULL && root[i]->getName()!=NULL) {
			if(!strcmp(name,root[i]->getName())){
				return root[i];
			}
		}
	}
	return NULL;
}

JSONObj *PIDs::jsonify(){
	JSONObj *json=new JSONObj();
	JSONArray *jsonarray=new JSONArray();
	for(int i=0;i<pidCount;i++) {
		if (root[i]!=NULL) {
			jsonarray->addElement(root[i]->jsonify());
		}
	}
	json->addElement("PIDs",jsonarray);
	return json;
}

PIDSTORObj *PIDS::storeify(){
	PIDSTORObj *store=new PIDSTORObj(pidCount);
	store->pidCount=pidCount;
	for (int i=0;i<pidCount;i++) {
        deventity *child=store->pids[i];
        if (root[i]!=NULL) {
        	root[i]->storeify(child);
        }
	}
	return store;
}


