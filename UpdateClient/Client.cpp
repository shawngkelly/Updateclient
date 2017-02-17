#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include "FileHelper.h"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

const char FILENAME[] = "data.bin";
const char IPADDR[] = "127.0.0.1";
const int  PORT = 50000;
const int  QUERY = 1;
const int  UPDATE = 2;
const int STRLEN = 256;

// Closes the socket and performs the WSACleanup

void cleanup(SOCKET socket);

// Returns the version number from the data file
int getLocalVersion();


// Reads the 3 data values from the data file.
// When the function ends, num1, num2 and num3 will be holding the
// 3 values that were read from the file.

void readData(int& num1, int& num2, int& num3);

//Client will start. It will check it's own version. Client will attempt to make connection to server.
//Once the client checks it's version against server, it will either request an update or close the connection.

int main()
{
	//Update vars
	int			sum;
	int			num1 = 0;
	int			num2 = 0;	
	int			num3 = 0;
	int			localVersion;

	//Socket vars
	WSADATA		wsaData;
	SOCKET		mySocket;
	SOCKADDR_IN	serverAddr;
	int		    recvMessage;
	bool		done = false;


	//Client checks the local version number

	localVersion = getLocalVersion();

	//Connection functions 
	// Loads Windows DLL (Winsock version 2.2) used in network programming

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		cerr << "ERROR: Problem with WSAStartup\n";
		return 1;
	}

	// Create a new socket for communication

	mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (mySocket == INVALID_SOCKET)
	{
		cerr << "ERROR: Cannot create socket\n";
		WSACleanup();
		return 1;
	}

	cout << "\nAttempting to connect...\n";

	// Setup a SOCKADDR_IN structure which will be used to hold address
	// and port information for the server. Notice that the port must be converted
	// from host byte order to network byte order.

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IPADDR, &serverAddr.sin_addr);

	// Try to connect

	if (connect(mySocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR: Failed to connect\n";
		cleanup(mySocket);
		return 1;
	}

	cout << "Connected...\n\n";

	//Send request to server for version number. 

	cout << "Sending request to server for version.\n\n " << localVersion << " is the client verison.\n";
	int isend = send(mySocket, (char*)&QUERY, sizeof((char*)&QUERY), 0);
	if (isend == SOCKET_ERROR)
	{
		cerr << "ERROR : FAILED TO SEND REQUEST\n";
		cleanup(mySocket);
		return 1;
	}

	if (!done)
	{
		// Wait to receive a reply message back from the remote computer

		cout << "----WAITING FOR REPLY ----\n\n";
		int iRecv = recv(mySocket, (char*)&recvMessage, sizeof((char*)&recvMessage), 0);
		if (iRecv > 0)
		{
			//Compares versions. If different will request new version. If same will close. 

			if(localVersion == recvMessage)
			{
				cout << "Client version " << localVersion << " is current with server version\n\n";
			}
			if(localVersion != recvMessage)
			{
				
				cout << "Server version " << recvMessage << " is incompatible with client version " << localVersion << "\n\n";

				 // Create a new socket to reconnect
				mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (mySocket == INVALID_SOCKET)
				{
					cerr << "ERROR: Cannot create socket\n";
					WSACleanup();
					return 1;
				}

				cout << "\nAttempting to reconnect...\n";  

				//Attempt to reconnect to reequest new version
				if (connect(mySocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
				{
					cerr << "ERROR: Failed to connect\n";
					cleanup(mySocket);
					return 1;
				}

				cout << "Reconnected. Sending request for update\n\n";  

				//Send a request for updated file. int 2

				if	(send(mySocket, (char*)&UPDATE, sizeof((char*)&UPDATE), 0) == SOCKET_ERROR)
				{
					cerr << "ERROR : FAILED TO SEND REQUEST\n";
					cleanup(mySocket);
					return 1;
				}

				//Waiting for file

				cout << "----Waiting for file-----\n\n";
				int recvNum1 = recv(mySocket, (char*)&num1, sizeof((char*)&num1), 0);
				int recvNum2 = recv(mySocket, (char*)&num2, sizeof((char*)&num2), 0);
				int recvNum3 = recv(mySocket, (char*)&num3, sizeof((char*)&num3), 0);
				if (recvNum1 == SOCKET_ERROR || recvNum2 == SOCKET_ERROR || recvNum3 ==SOCKET_ERROR)
				{
					cerr << "ERROR : FAILED TO RCV UPDATE\n";
					cleanup(mySocket);
					return 1;
				}

				//Open file and write date to file. 

				cout << "Update received from server. \n";
				ofstream dataFile;
				openOutputFile(dataFile, FILENAME);
				writeInt(dataFile, num1);
				writeInt(dataFile, num2); 
				writeInt(dataFile, num3);
				dataFile.close();
			}
			  

			cout << "Version is current\n\n";
			cout << "\nSum Calculator Version " << getLocalVersion() << "\n\n";

			readData(num1, num2, num3);
			sum = num2 + num3;
			cout << "The sum of " << num2 << " and " << num3 << " is " << sum << endl;

			cleanup(mySocket);
			return 0;			
		}

		else if (iRecv == 0)
		{
			cout << "Connection closed\n";
			cleanup(mySocket);
			return 0;
		}

		else
		{
			cerr << "ERROR: Failed to receive message\n";
			cleanup(mySocket);
			return 1;
		}

	}


	cleanup(mySocket);
	return 0;
}

int getLocalVersion()
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	int version = readInt(dataFile);
	dataFile.close();

	return version;
}

void readData(int& num1, int& num2, int& num3)
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	// Read the version number and discard it
	  num1 = readInt(dataFile);

	// Read the two data values
	num2 = readInt(dataFile);
	num3 = readInt(dataFile);

	dataFile.close();
}

void cleanup(SOCKET socket)
{
	closesocket(socket);
	WSACleanup();
}