#ifndef BREWLINK_H
#define BREWLINK_H
#include "testing.h"
#ifndef XCODE
#include "application.h"
#endif
#include <stdarg.h>
#include <vector>
#include <algorithm>  //provides remove_if

#pragma once

#define TCPTIMEOUT 60000 //default timeout is 60 seconds.

String charToString(char *cstr);

// linked list of attached clients
typedef struct BrewLinkClient{
	unsigned long lastHeard; //last time we heard from this client from millis
	TCPClient client;
	char cmdBuf[256];
	int bufPos;

} BrewLinkClient;


class BrewLink{
public:
	BrewLink();
	~BrewLink();
	void begin();
	void stop();
	void receive();
	int print(char *buf, int buflen, BrewLinkClient *client);
	void printDebug(const char *format, ...);
	char *processCmd(char *cmd);
	char *cmdStatus();
	char *cmdDeviceSearch();
	char *cmdToggle(char *cmd);
	char *cmdSet(char *cmd);
	char *cmdConn(char *cmd);
private:
	//Photon has a global 'Serial' which we'll use instead of a private var here for serial communications
	TCPServer *wifiserver; //we'll also listen for clients on wifi
	//BrewLinkClient *clients;
	std::vector<BrewLinkClient> clients; //TCPServer clients

	//TCPClient singleClient;
	//unsigned long lastHeard; //last time we heard from singleClient
	char serialCmdBuf[256];
	int serialBufPos;
	//char singleClientCmdBuf[256];
	//int singleClientBufPos;
	void receiveTCPClient(BrewLinkClient *client);
};

#endif
