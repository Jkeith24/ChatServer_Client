// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ClientClass.h"

//LEAK DETECTION
#if defined _MSC_VER && defined _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define ENABLE_LEAK_DETECTION() _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)        //went on a stroll to memory lane at fullsail to find this leak detection
#else
#define ENABLE_LEAK_DETECTION()
#endif


int main() {

    ENABLE_LEAK_DETECTION();   

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed");
             
        return 1;
    }

    ClientClass client;


    client.listenForUDPPackets(45612);
    //waiting for UDP information to auto connect to server
  



   /* std::cout << "Enter the server IP address: ";
    std::cin >> client.serverIP;

    std::cin.ignore(1000, '\n');


    std::cout << "Enter the server port: ";
    std::cin >> client.serverPort;*/

   /* std::cin.ignore(1000, '\n');*/

    std::cout << "Please enter your username: ";
    std::cin.getline(client.username, 50) ;   

    system("cls");
    
    char registerCommand[] = "$register ";
    std::string exitCommand = "$exit";
    std::string getListCommand = "$getlist";
    std::string getLogCommand = "$getlog";

    


    int errorCheck = client.init(client.serverPort, client.serverIP);

    if (errorCheck != SUCCESS)
    {
        printf("Initialization error\n");

       
        delete[] client.username;
        delete[] client.serverIP;
        return 1;
    }

    std::thread listenForServerMessages([&]() {client.gettingServerData(); });
   

    int combinedLength = strlen(registerCommand) + strlen(client.username) + 1; //+1 for null terminator

    char* registerMessage = new char[combinedLength];

    strcpy(registerMessage, registerCommand);
    strcat(registerMessage, client.username);

    
    client.sendMessage(registerMessage, combinedLength);

    char* userMessages = new char[255];

    //char serverMessage[255] = { 0 };   


    fd_set readSet;
    FD_ZERO(&readSet); // Add stdin (user input) to the set
    FD_SET(client.clientSocket, &readSet); // Add server socket to the set

  

   //using multiplexing in another thread to recieve server messages so user input doesn't interfere with recieving server messages

    //while loop to send messages to server
    while (client.runClient)
    {
       
        userMessages[0] = '\0';

        std::cin.getline(userMessages, 255);
        std::cout << client.username << ": " << userMessages;     


        std::string exit = userMessages;

        if (exit.find(exitCommand) != std::string::npos)
        {
            std::cout << "Closing socket connection to server\n";
            closesocket(client.clientSocket);            
            client.runClient = false;
           
            break;
        }
        else if (exit.find(getLogCommand) != std::string::npos)
        {

            client.sendMessage(userMessages, strlen(userMessages) + 1);
            std::cout << '\n';

        }        
        else
        {
            client.sendMessage(userMessages, strlen(userMessages) + 1);
            std::cout << '\n';

        }

    }

    //cleaning up memory 

    listenForServerMessages.join();


    client.runClient = false;
    delete[] registerMessage;
    delete[] userMessages;
    delete[] client.username;
    delete[] client.serverIP;

    WSACleanup();

    return 0;
}