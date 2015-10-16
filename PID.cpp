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
}

PID::~PID() {

}

void PID::setSetPoint(double newtemp) {
	setPoint=newtemp;
}

char *PID::getName(){
	if(dev!=NULL) {
		return dev->getName();
	}
	return NULL;
}

void PID::update(){
	unsigned long stime=millis();
	temp=dev->getValue();
#ifndef XCODE
    //bLink->printDebug("get Temp took %lu milliseconds.",millis()-stime);
#else
    printf("get Temp took %lu milliseconds.\n",millis()-stime);
#endif
	double P=setPoint-temp;
	if (state!=IDLE && ((P>=0 && P<=deadBand) || (P<0 && (-P)<=deadBand))) {
		state=IDLE;
		PIDvalue=0;
		I=0;
	} else {
		if(P>0){
			state=HEATING;
		} else {
			state=COOLING;
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
	json->addElement("heating",heating());
	json->addElement("cooling",cooling());
	json->addElement("deadBand",deadBand);
	json->addElement("PWMScale",PWMScale);
	json->addElement("PWMDutyCycle",PWMDutyCycle());
	return json;
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
	unsigned long stime=millis();
	for(int i=0;i<pidCount;i++) {
		if (root[i]!=NULL) {
			root[i]->update();
			//bLink->printDebug("updated PID %s",root[i]->getName());
		}
	}
#ifndef XCODE
	//bLink->printDebug("updatePIDs took %lu milliseconds.",millis()-stime);
#else
    printf("updatePIDs took %lu milliseconds.\n",millis()-stime);
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


