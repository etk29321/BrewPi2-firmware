#ifndef PID_H
#define PID_H
#include <stdlib.h>
#include <math.h>
#include "Brewpi.h"
#include "JSON.h"
#include "Storage.h"

#pragma once
#ifndef PIDenum
#define PIDenum
enum PIDState{
	IDLE=0,
	COOLING=1,
	HEATING=2,
};
#endif
/* BrewPi PID defaults
   Kp	 doubleToTempDiff(5.0),	 +5
   Ki	 doubleToTempDiff(0.25),  +0.25
   Kd	 doubleToTempDiff(1.5),	+1.5 */
class PID{
public:
	PID(class Device *newdev, double P=5, double I=0.25, double D=1.5, double dBand=0, double PWMScalein=1);
	~PID();
	void setSetPoint(double temp);
	void update();
	bool heating();
	bool cooling();
	char *getName();
	String getStrName();
	uint8_t PWMDutyCycle();
	JSONObj *jsonify();
	void storeify(pidentity *pident);
	void setP(double newP);
	void setI(double newI);
	void setD(double newD);
	void setMinStateTime(unsigned long newMinStateTime);
	void setDeadBand(double newBand);
	void setPWMScale(double newScale);
private:
	double Kp;
	double Ki;
	double Kd;
	double I;
	double prevError;
	double setPoint;
	double temp;
	double PIDvalue;
	unsigned long prevTime;
	double deadBand;
	bool warm;
	bool cool;
	class Device *dev;
	double PWMScale;
	PIDState state;
	unsigned long lastStateChange;
	unsigned long minStateTime;
	unsigned long stateTime;

};

class PIDs {
public:
	PIDs();
	~PIDs();
	void addPID(PID *p);
	void updatePIDs();
	PID *getPID(char *name); //PID name is the dev->getName() of the underlying device
	JSONObj *jsonify();
	PIDSTORObj *storeify();
private:
	PID **root;
	int pidCount;
};
#endif
