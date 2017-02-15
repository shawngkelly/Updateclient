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




// Reads the two data values from the data file.
// When the function ends, num1 and num2 will be holding the
// two values that were read from the file.
void readData(int& num1, int& num2, int& num3);

int main()
{
	//Update vars
	int			sum;
	int			num1 = 0;
	int			num2 = 0;	
	int			num3 = 0;
	int			localVersion = 0;

	//Socket vars
	WSADATA		wsaData;
	SOCKET		mySocket;
	SOCKADDR_IN	serverAddr;
	char		ipAddress[20];
	int			port;
	char		sendMessage[STRLEN];
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

	// Add code here to
	// 1) make sure that we are using the current version of the data file
	// 2) update the data file if it is out of data
	
	// Main purpose of the program starts here: read two numbers from the data file and calculate the sum
	
	cout << "\nSum Calculator Version " << localVersion << "\n\n";

	readData(num1, num2, num3);	
	sum = num2 + num3;
	cout << "The sum of " << num2 << " and " << num3 << " is " << sum << endl;
	
	//Send request to server

	cout << " Checking with server for current version number " << localVersion;
	send(mySocket, (char*)&QUERY, sizeof((char*)&QUERY), 0);


	if (!done)
	{
		// Wait to receive a reply message back from the remote computer

		cout << "\n\t--WAIT--\n\n";
		int iRecv = recv(mySocket, (char*)&recvMessage, sizeof((char*)&recvMessage), 0);
		if (iRecv > 0)
		{
			//Compares versions. If different will request new version. If same will close. 

			if (localVersion != recvMessage)
			{
				
				cout << "Server version " << recvMessage << " is incompatible with Local version " << localVersion << "\n\n";

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

				cout << "Reconnected. Send request for update\n\n";  

				//Send a request for updated file. int 2
				send(mySocket, (char*)&UPDATE, sizeof((char*)&UPDATE), 0);

				//Waiting for file
				cout << "---Waiting for file";
				int recvNum1 = recv(mySocket, (char*)&num1, sizeof((char*)&num1), 0);
				int recvNum2 = recv(mySocket, (char*)&num2, sizeof((char*)&num2), 0);
				int recvNum3 = recv(mySocket, (char*)&num3, sizeof((char*)&num3), 0);
				if ((recvNum1 || recvNum2 || recvNum3) == SOCKET_ERROR)
				{
					cerr << "ERROR : FAILED TO RCV UPDATE\n";
					cleanup(mySocket);
				}
				cout << "Update received from server. \n";
				ofstream dataFile;
				openOutputFile(dataFile, FILENAME);
				writeInt(dataFile, num1);
				writeInt(dataFile, num2); 
				writeInt(dataFile, num3);
				dataFile.close();


			}
			else if (localVersion == recvMessage)
			{   
				cout << "Version is current\n\n";
				cout << "\nSum Calculator Version " << localVersion << "\n\n";

				readData(num1, num2, num3);
				sum = num2 + num3;
				cout << "The sum of " << num2 << " and " << num3 << " is " << sum << endl;
				cleanup(mySocket);

				return 0;
			}

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