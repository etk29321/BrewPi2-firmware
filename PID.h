#ifndef PID_H
#define PID_H
#include <stdlib.h>
#include <math.h>
#include "DeviceManager.h"
#include "JSON.h"

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
   Kd	 doubleToTempDiff(1.5),	-1.5 */
class PID{
public:
	PID(class Device *newdev, double P=5, double I=0.25, double D=-1.5, double dBand=0, double PWMScalein=1);
	~PID();
	void setSetPoint(double temp);
	void update();
	bool heating();
	bool cooling();
	char *getName();
	uint8_t PWMDutyCycle();
	JSONObj *jsonify();
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
};

class PIDs {
public:
	PIDs();
	~PIDs();
	void addPID(PID *p);
	void updatePIDs();
	PID *getPID(char *name); //PID name is the dev->getName() of the underlying device
	JSONObj *jsonify();
private:
	PID **root;
	int pidCount;
};
#endif
