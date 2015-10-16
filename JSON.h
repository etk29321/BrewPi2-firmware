#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testing.h"
#ifndef XCODE
#include <application.h> //bring in String
#endif
#pragma once

class JSONArray;
class JSONArrayElement;
class JSONObj;
class JSONElement;

class JSONArrayElement{
public:
	JSONArrayElement(const char *newval);
	JSONArrayElement(char *newval);
	JSONArrayElement(double newval);
	JSONArrayElement(JSONObj *newval);
	JSONArrayElement(JSONArray *newval);
	~JSONArrayElement();
	char *getValue();
	JSONObj *getValueAsObj();
	JSONArray *getValueAsArray();
	double getValueAsDouble();
	void setValue(const char *newval);
	void setValue(char *newval);
	void setValue(JSONObj *newval);
	void setValue(JSONArray *newval);
	void setValue(double newval);
	char *jstringify(); //latest photon has a public method called stringify, so we can't use that name.
private:
	JSONObj *obj;
	JSONArray *array;
	char *val;
	double numval;
};

class JSONArray{
public:
	JSONArray();
	JSONArray(char *doc);
	~JSONArray();
	void addElement(const char *newval);
	void addElement(char *newval);
	void addElement(double newval);
	void addElement(JSONObj *newval);
	void addElement(JSONArray *newval);
	void addElement(JSONArrayElement *newval);
	bool getNextElement(JSONArrayElement **element);
	void resetPos();
	char *jstringify();
private:
	JSONArrayElement **root;
	int len;
	char *_nexttoken(char *data);
	int getpos;
};

class JSONElement{
public:
	JSONElement(char *doc);
	JSONElement(const char *elementName,const char *newval);
	JSONElement(const char *elementName,char *newval);
	JSONElement(const char *elementName,double newval);
	JSONElement(const char *elementName,JSONObj *newval);
	JSONElement(const char *elementName,JSONArray *newval);
	~JSONElement();
	char *getValue();
	int getValueAsInt();
	JSONArray *getValueAsArray();
	JSONObj *getValueAsObj();
	char *getName();
	void setValue(char *newval);
	void setValue(JSONObj *newval);
	void setValue(JSONArray *newval);
	void setValue(double newval);
	char *jstringify();
private:
	JSONObj *obj;
	JSONArray *array;
	char *val;
	double numval;
	char *name;
};


class JSONObj{
public:
	JSONObj();
	JSONObj(char *doc);
	~JSONObj();
	JSONElement *getElement(char *name);
	JSONElement *getFirstElement();
	bool getNextElement(JSONElement **element);
	void resetPos();
	void addElement(JSONElement *newElement);
	void addElement(const char *elementName,char *newval);
	void addElement(const char *elementName,double newval);
	void addElement(const char *elementName,JSONObj *newval);
	void addElement(const char *elementName,JSONArray *newval);
	char *jstringify();
private:
	JSONElement **root;
	int len;
	char *_nexttoken(char *data);
	int getpos;

};

typedef class JSONObj JSONObj;


#endif
