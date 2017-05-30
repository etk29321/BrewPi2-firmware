#ifndef CONNECT_H
#define CONNECT_H

#include "Brewpi.h"
#include "JSON.h"
#include "PID.h"

enum ConnMode {
	PIDCooling=0,
	PIDHeating=1,
	DevMode=2,
	Custom=3,
};

#ifndef PIDenum
#define PIDenum
enum PIDState{
	IDLE=0,
	COOLING=1,
	HEATING=2,
};

#endif

class Connection{
public:
	Connection(class Device *outdev, char *exp);
	Connection(class Device *outdev, Device *indev);
	Connection(class Device *outdev, class PID *inpid,PIDState state);
	~Connection();
	void update();
	char *getName(); //output device name this connection goes to
	JSONObj *jsonify();
	Connection *next;
	Connection *prev;
	class Device *getOutDev();
	class Device *getInDev();
	class PID *getInPID();
	char *getExp();
	PIDState getPIDState();
private:
	bool eval(char *exp);
	bool evalToken(char *exp);
	char *_nexttoken(char *exp);
	class PID *pid;
	class Device *inputdev;
	class Device *dev;
	int prevOutput;
	char *expression;
	ConnMode mode;
	bool firstRun;

};

class Connections {
public:
	Connections();
	~Connections();
	void addConnection(JSONObj *json);
	void addConnection(Connection *newconn);
	void delConnection(JSONObj *json); //Device name is the dev->getName() of the underlying device
	void delConnection(char *name); //Device name is the dev->getName() of the underlying device
	void update();
	Connection *getConnection(char *name); //Device name is the dev->getName() of the underlying device
	Connection *getConnection(int connum); //get connection by index
	int getNumConns();
	JSONObj *jsonify();
private:
	Connection *root;
	int connCount;
};

#endif
