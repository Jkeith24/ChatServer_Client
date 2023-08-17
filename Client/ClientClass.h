#pragma once


#include "definitions.h"
#include <iostream>
#include <string>
#include <thread>



class ClientClass {

	bool isRegistered = false;

public:

	const int bufferSize = 50;
	bool runClient = true;

	char* serverIP = new char[bufferSize];
	char* username = new char[bufferSize];

    uint16_t serverPort;
    SOCKET clientSocket;	


	int init(uint16_t port, const char* address);
	int readMessage(SOCKET _socket, char* buffer, int32_t size);
	int sendMessage(char* data, int32_t length);
	void stop();

	int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
	int tcp_recv_whole(SOCKET s, char* buf, int len);

	int gettingServerData();

};

