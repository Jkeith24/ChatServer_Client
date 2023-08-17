#include "ClientClass.h"
#undef max



int ClientClass::init(uint16_t port, const char* address)
{
	int error = 0;
	unsigned long ulAddr = inet_addr(address);

	if (ulAddr == INADDR_NONE)
	{
		printf("Server Address error");
		return ADDRESS_ERROR;
	}

	SOCKET ComSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (ComSocket == INVALID_SOCKET)
	{
		printf("Invalid Socket error");
		return SETUP_ERROR;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.S_un.S_addr = inet_addr(address);
	serverAddress.sin_port = htons(port);

	int result = connect(ComSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));

	if (result == SOCKET_ERROR)
	{
		printf("\nconnect failed\n");
		error = WSAGetLastError();
		if (error == 10047)
		{
			return CONNECT_ERROR;
		}
		else if (error == 10061)
		{
			return CONNECT_ERROR;
		}

		return SHUTDOWN;

	}

	if (result == SHUTDOWN)
	{
		return SHUTDOWN;
	}

	clientSocket = ComSocket;

	return SUCCESS;
}

int ClientClass::readMessage(SOCKET _socket, char* buffer, int32_t size)
{
	int error;

	if (buffer == nullptr)
	{
		return PARAMETER_ERROR;
	}

	uint8_t messageSize = 0;

	int result = tcp_recv_whole(clientSocket, (char*)&messageSize, 1);

	if (messageSize > size)
	{
		return PARAMETER_ERROR;
	}

	if (result == 0)
	{
		return SHUTDOWN;
	}

	if (result == SOCKET_ERROR)
	{
		error = WSAGetLastError();

		if (error >= 6 && error <= 11031) //just doing a range of errors due to not knowing what error codes could be possible
		{
			return DISCONNECT;

		}
		else
		{
			return SHUTDOWN;
		}


	}

	result = tcp_recv_whole(clientSocket, buffer, messageSize);

	if (result == 0)
	{
		return SHUTDOWN;
	}

	if (result == SOCKET_ERROR)
	{
		error = WSAGetLastError();

		if (error >= 6 && error <= 11031) //just doing a range of errors due to not knowing what error codes could be possible
		{
			return DISCONNECT;

		}
		else
		{
			return SHUTDOWN;
		}

	}


	return SUCCESS;
}


int ClientClass::sendMessage(char* data, int32_t length)
{

	int error = 0;

	if (data == nullptr)
	{
		return PARAMETER_ERROR;
	}

	if (length < 0 || length > 255) {
		return PARAMETER_ERROR;
	}

	uint8_t size = length;
	char* sendBuffer = new char[size];
	memset(sendBuffer, 0, length);


	strcpy(sendBuffer, data);

	int result = tcp_send_whole(clientSocket, (char*)&size, 1);

	if (result == SOCKET_ERROR)
	{
		error = WSAGetLastError();

		if (error >= 6 && error <= 11031) //just doing a range of errors due to not knowing what error codes could be possible
		{
			return DISCONNECT;

		}
		else
		{
			return SHUTDOWN;

		}

	}

	result = tcp_send_whole(clientSocket, data, size);
	if (result == SOCKET_ERROR)
	{
		error = WSAGetLastError();

		if (error >= 6 && error <= 11031) //just doing a range of errors due to not knowing what error codes could be possible
		{
			return DISCONNECT;

		}
		else
		{
			return SHUTDOWN;

		}
	}

	delete[] sendBuffer;

	return SUCCESS;
}

void ClientClass::stop()
{
	shutdown(clientSocket, 2);
	closesocket(clientSocket);
}

int ClientClass::tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
	int result;
	int bytesSent = 0;

	while (bytesSent < length)
	{
		result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}

int ClientClass::tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;

	do
	{
		int ret = recv(s, buf + total, len - total, 0);
		if (ret < 1)
			return ret;
		else
			total += ret;

	} while (total < len);

	return total;
}

int ClientClass::gettingServerData()
{

	fd_set readSet{};
	FD_SET(clientSocket, &readSet); // Add server socket to the set

	char serverMessage[255] = { 0 };

	while (runClient)
	{
		fd_set tmpSet = readSet;
		int maxFd = std::max(0, (int)clientSocket) + 1;

		int selectResult = select(maxFd, &tmpSet, nullptr, nullptr, nullptr);

		if (selectResult == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			std::cout << error;

			printf("\nSELECT ERROR\n");
			runClient = false;
			break;
		}

		//read messages
		if (FD_ISSET(clientSocket, &tmpSet))
		{
			//data is ready from the server
			int readresult = readMessage(clientSocket, serverMessage, 255);

			if (readresult == SOCKET_ERROR)
			{
				printf("SOCKET ERROR WHILE READING MESSAGES");
				FD_CLR(clientSocket, &readSet);
				shutdown(clientSocket, 2);			//removing and cleaning up a socket
				closesocket(clientSocket);
				runClient = false;
				break;

			}
			else if (readresult == 2)		//ECONRESET 10054
			{
				printf("ERROR: SERVER closed the connection. The Server is likely full if this happens immedietely on connect!");
				FD_CLR(clientSocket, &readSet);
				shutdown(clientSocket, 2);			//removing and cleaning up a socket
				closesocket(clientSocket);
				runClient = false;
				break;

			}

		}
		if (!isRegistered)
		{
			if (serverMessage == "SV_FULL")
			{
				std::cout << "ERROR: SERVER IS FULL";
				runClient = false;
				shutdown(clientSocket, 2);
				closesocket(clientSocket);
			}
			else if (serverMessage == "SV_SUCCESS")
			{
				std::cout << "SUCCESSFULLY CONNECTED TO SERVER!";
				isRegistered = true;
			}
		}

		std::cout << '\n' << serverMessage << '\n';

	}

	return 0;
}
