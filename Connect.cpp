#include "Connect.h"

Connection::Connection(Device *outdev, char *exp){
	dev=outdev;
	prevOutput=LOW;
	size_t explen=strlen(exp)+1;
	expression=(char *)malloc(sizeof(char *)*explen);
	memcpy(expression,exp,explen);
	pid=NULL;
	inputdev=NULL;
	mode=Custom;
	next=NULL;
	prev=NULL;
	firstRun=true;
}

Connection::Connection(Device *outdev, Device *indev){
	dev=outdev;
	prevOutput=LOW;
	expression=NULL;
	pid=NULL;
	inputdev=indev;
	mode=DevMode;
	next=NULL;
	prev=NULL;
	firstRun=true;

}

Connection::Connection(Device *outdev, PID *inpid, PIDState state){
	dev=outdev;
	prevOutput=LOW;
	expression=NULL;
	pid=inpid;
	inputdev=NULL;
	if (state==HEATING) {
		mode=PIDHeating;
	} else {
		mode=PIDCooling;
	}
	next=NULL;
	prev=NULL;
	firstRun=true;

}

Connection::~Connection(){
	if(expression!=NULL) {
		free(expression);
	}
	pid=NULL;
	inputdev=NULL;
	dev=NULL;
	expression=NULL;
	next=NULL;
	prev=NULL;
}

void Connection::update(){

	bool input=false;
	switch(mode) {
	case PIDCooling:
		if (pid==NULL) {
			bLink->printDebug("ERROR: Connection::update PIDCooling called with NULL pid");
			return;
		}
		input=pid->cooling();
		break;
	case PIDHeating:
		if (pid==NULL) {
			bLink->printDebug("ERROR: Connection::update PIDHeating called with NULL pid");
			return;
		}
		input=pid->heating();
		break;
	case DevMode:
		if (dev==NULL) {
			bLink->printDebug("ERROR: Connection::update Dev called with NULL dev");
			return;
		}
		input=dev->getValue();
		break;
	case Custom:
		//bLink->printDebug("evaling expression %s",expression);
		if (expression==NULL) {
			bLink->printDebug("ERROR: Connection::update Custom called with NULL expression");
			return;
		}
		input=eval(expression);
		//bLink->printDebug("completed eval of %s=%d",expression,input);
		break;
	}
	//bLink->printDebug("input=%d prevOutput=%d",input,prevOutput);
	if(firstRun) {
		dev->setOutput(input);
		prevOutput=input;
		firstRun=false;
	} else {
		if(input!=prevOutput) {
			dev->setOutput(input);
			prevOutput=input;
		}
	}
}

/*
 *
 *  & - and
 *  | - or
 *  ! - not
 *  ^ - xor
 *  [H = PID heating
 *  [C = PID cooling
 *  { = Device
 *
 *  Example: [Cbeertemp & !{dooropen
 *  This will set the device output to high if the PID 'beertemp' is calling for cooling and the device named 'dooropen' is not HIGH
 *
 */

bool Connection::eval(char *exp) {
	char *pos=exp;
	bool a;
	bool b;
	char *prevToken=NULL;
	char *token=NULL;
	while (*pos!='\0' && *pos!=')') {
		//bLink->printDebug("eval:pos=%c",*pos);
		token=_nexttoken(pos);
		pos+=strlen(token);
		while (*pos==' '||*pos=='\t'||*pos=='\n' ||*pos=='\r') { //skip whitespace
			pos++;
		}
		switch (*pos){
		case '&': //and
			prevToken=token;
            pos++;
            token=_nexttoken(pos);
			a=evalToken(prevToken);
			b=evalToken(token);
			free(prevToken);
			free(token);
			return a&b;
			break;
		case '|': //or
			prevToken=token;
            pos++;
			token=_nexttoken(pos);
			a=evalToken(prevToken);
			b=evalToken(token);
			free(prevToken);
			free(token);
			return a|b;
			break;
		case '!': //not
			free(token);
            pos++;
			token=_nexttoken(pos);
			a=evalToken(token);
			free(token);
			return !a;
			break;
		case '^': //xor
			prevToken=token;
            pos++;
			token=_nexttoken(pos);
			a=evalToken(prevToken);
			b=evalToken(token);
			free(prevToken);
			free(token);
			return a^b;
			break;
		//default:
			// Invalid token
			//bLink->printDebug("Invalid pos %c |%s| exp=%s",*pos,pos,exp);
		}
		//if (prevToken!=NULL) {free(prevToken);}
	}
	if(token!=NULL) {
		return evalToken(token);
	}
	return false; //default to off.  We should never get here.
}

bool Connection::evalToken(char *exp) {
	Device *tokendev;
	PID *tokenpid;
	switch (*exp) {
	case '(':
		return eval(exp+1);
		break;
	case '[':
        if ( *(exp+1)!='\0' && *(exp+2)!='\0') {
            tokenpid=pids->getPID(exp+2);
            if(tokenpid!=NULL) {
                switch (*(exp+1)) {
                case 'H':
                        return tokenpid->heating();
                        break;
                    case 'C':
                        return tokenpid->cooling();
                        break;
                }
            }
        }
		break;
	case '{':
		tokendev=deviceManager->getDevice(exp+1);
		if(tokendev!=NULL) {
			//bLink->printDebug("evalToken: %s value=%s",exp,String(tokendev->getValue(), 2).c_str());
			return tokendev->getValue();
		}
	}
	return true;
}

char *Connection::_nexttoken(char *exp) {
		char *token=(char *)malloc(sizeof(char)*(strlen(exp)+1));
		//if (token==NULL) {
		//	bLink->printDebug("MALLOC FAILED");
		//}
		char *bufpos=token;
		char *pos=exp;
		*token='W';
        int nestcount=0;
		while ((*pos==' ' ||*pos=='\t' ||*pos=='\n' ||*pos=='\r')) { //scan forward to start of token
			pos++;
			}
		while (((*pos!=')' && *pos!='&' && *pos!='|' && *pos!='!' && *pos!='^') || nestcount>0) && *pos!='\0') {
            switch(*pos) {
                case '(':
                    nestcount++;
                    *bufpos=*pos;
        			//bLink->printDebug("_nexttoken:pos=%c nestcount=%d buf=%s",*bufpos, nestcount, token);
                    bufpos++;
                    break;
                case ')':
                    nestcount--;
                    *bufpos=*pos;
        			//bLink->printDebug("_nexttoken:pos=%c nestcount=%d buf=%s",*bufpos, nestcount, token);
                    bufpos++;
                    break;
                case ' ':  //skip whitespace
                case '\n':
                case '\t':
                case '\r':
                    break;
                default:
                    *bufpos=*pos;
        			//bLink->printDebug("_nexttoken:pos(0x%x)=%c nestcount=%d buf(0x%x)=%s",(int)bufpos, *bufpos, nestcount, (int)token, token);
                    bufpos++;
                    break;
            }
			pos++;
		}

        *bufpos='\0'; //terminate string
    //bLink->printDebug("Next token=|%s|",token);
#ifdef XCODE
        printf("Next token=>%s<\n",token);
#else
        //bLink->printDebug("Next token=>%s<",token);
#endif
		return token;
}

char *Connection::getName() {
	if(dev!=NULL) {
		return dev->getName();
	}
	return NULL;
}


class Device *Connection::getOutDev(){
	return dev;
}
class Device *Connection::getInDev(){
	return inputdev;
}
class PID *Connection::getInPID(){
	return pid;
}
ConnMode Connection::getMode(){
	return mode;
}


char *Connection::getExp(){
	return expression;
}
PIDState Connection::getPIDState(){
	if (mode==PIDHeating) {
		return HEATING;
	} else {
		return COOLING;
	}
}


JSONObj *Connection::jsonify(){
	JSONObj *json=new JSONObj();
	json->addElement("OutputDevice",dev->getName());
	json->addElement("mode",mode);
	switch (mode) {
	case PIDCooling:
	case PIDHeating:
		json->addElement("PID",pid->getName());
		break;
	case DevMode:
		json->addElement("InputDevice",dev->getName());
		break;
	case Custom:
		json->addElement("Expression",expression);
		break;
	}
	return json;
}

/******************************************************************************
 *
 *  Connections Object
 *
 *
 *****************************************************************************/

Connections::Connections(){
	root=NULL;
	connCount=0;
}
Connections::~Connections(){
	Connection *conn=root;
	Connection *nextconn;
	while(conn!=NULL) {
		nextconn=conn->next;
		delete conn;
		conn=nextconn;
	}
}
void Connections::addConnection(Connection *newconn){
	if(newconn!=NULL) {
		if(root==NULL) {
			root=newconn;
			connCount++;
		} else {
			Connection *checkExists=getConnection(newconn->getName());
			if(checkExists==NULL) { //make sure we don't add a connection if one already exists for the base device
				Connection *newroot=newconn;
				newconn->next=root;
                root->prev=newconn;
				root=newroot;
            } else { //overwrite any existing connection
            	delConnection(newconn->getName());
            	addConnection(newconn);
            }
		}
	}
}

void Connections::addConnection(JSONObj *json){
	JSONElement *jsonOutDev=json->getElement("OutputDevice");
	JSONElement *jsonmode=json->getElement("mode");
	JSONElement *jsonInDev=json->getElement("InputDevice");
	JSONElement *jsonPID=json->getElement("PID");
	JSONElement *jsonExp=json->getElement("Expression");
	Connection *newconn=NULL;

	if(jsonOutDev!=NULL && jsonmode!=NULL) {
		char *outputDevice=jsonOutDev->getValue();
		Device *dev=deviceManager->getDevice(outputDevice);
		if (dev!=NULL) {
			int mode=jsonmode->getValueAsInt();
			PID *temppid;
			Device *indev;
			char *exp;
			switch((int)mode){
			case 0:
				if(jsonPID!=NULL) {
					temppid=pids->getPID(jsonPID->getValue());
					if(temppid!=NULL) {
						newconn=new Connection(dev,temppid,COOLING);
					}
				}
				break;
			case 1:
				if(jsonPID!=NULL) {
					temppid=pids->getPID(jsonPID->getValue());
					if(temppid!=NULL) {
						newconn=new Connection(dev,temppid,HEATING);
					}
				}
				break;
			case 2:
				if(jsonInDev!=NULL) {
					indev=deviceManager->getDevice(jsonInDev->getValue());
					if (index!=NULL) {
						newconn=new Connection(dev,indev);
					}
				}
				break;
			case 3:
				if(jsonExp!=NULL){
					exp=jsonExp->getValue();
					if (exp!=NULL) {
						newconn=new Connection(dev,exp);
					}
				}
				break;
			}
		}
	}
	if (newconn!=NULL) {
		addConnection(newconn);
	}
}

void Connections::delConnection(JSONObj *json){ //device name is the dev->getName() of the underlying device
	JSONElement *jsonOutDev=json->getElement("OutputDevice");
	if(jsonOutDev!=NULL) {
		char *outputDevice=jsonOutDev->getValue();
		if(outputDevice!=NULL) {
			bLink->printDebug("deleting connection %s",outputDevice);
			delConnection(outputDevice);
		}
	}
}

void Connections::delConnection(char *name){ //device name is the dev->getName() of the underlying device
	Connection *conn=getConnection(name);
	if(conn!=NULL){
		bLink->printDebug("deleting connection 0x%x prev=0x%x next=0x%x",(int)conn,(int)conn->prev,(int)conn->next);
		if(conn->prev!=NULL) {
			conn->prev->next=conn->next;
		}
		if(conn->next!=NULL) {
			conn->next->prev=conn->prev;
		}
		bLink->printDebug("removed from list");
		if(conn==root) {
			root=conn->next;
		}
		delete conn;
		connCount--;
	}
	return;
}


void Connections::update(){
	Connection *conn=root;
	while(conn!=NULL) {
		//bLink->printDebug("Updateing connection 0x%x",(int)conn);
		conn->update();
		conn=conn->next;
	}
}



Connection *Connections::getConnection(char *name){ //device name is the dev->getName() of the underlying device
	Connection *pos=root;
	while(pos!=NULL) {
		if(!strcmp(pos->getName(),name)) {
			return pos;
		}
        pos=pos->next;
	}
	return NULL; //not found
}

Connection *Connections::getConnection(int connum){
	Connection *pos=root;
	int i=0;
	while(pos!=NULL && i<connum) {
        pos=pos->next;
	}
	return pos;
}

int Connections::getNumConns(){
	return connCount;
}



JSONObj *Connections::jsonify(){
	JSONObj *json=new JSONObj();
	JSONArray *jsonarray=new JSONArray();
	Connection *pos=root;
	while(pos!=NULL) {
		jsonarray->addElement(pos->jsonify());
        pos=pos->next;
	}
	json->addElement("Connections",jsonarray);
	return json;
}
