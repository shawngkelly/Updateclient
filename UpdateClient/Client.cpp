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
void readData(int& num1, int& num2);

int main()
{
	//Update vars
	int			sum;
	int			num1 = 0;
	int			num2 = 0;	
	int			localVersion = 0;

	//Socket vars
	WSADATA		wsaData;
	SOCKET		mySocket;
	SOCKADDR_IN	serverAddr;
	char		ipAddress[20];
	int			port;
	char		sendMessage[STRLEN];
	char		recvMessage[STRLEN];
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

	readData(num1, num2);	
	sum = num1 + num2;
	cout << "The sum of " << num1 << " and " << num2 << " is " << sum << endl;
	
	//Send request to server

	cout << " Checking with server for version number " << localVersion;
	send(mySocket, (char*)&QUERY, strlen((char*)&QUERY), 0);


	if (!done)
	{
		// Wait to receive a reply message back from the remote computer

		cout << "\n\t--WAIT--\n\n";
		int iRecv = recv(mySocket, recvMessage, STRLEN, 0);
		if (iRecv > 0)
		{
			//Compares versions. If different will request new version. If same will close. 

			if (localVersion != (int)recvMessage[0])
			{
				recvMessage[iRecv] = '\0';
				cout << "Server version " << (int)recvMessage[0] << " is incompatible with Local version " << localVersion << "\n\n";

			}
			else
			{   
				cout << "Version is current\n\n";
				cout << "\nSum Calculator Version " << localVersion << "\n\n";

				readData(num1, num2);
				sum = num1 + num2;
				cout << "The sum of " << num1 << " and " << num2 << " is " << sum << endl;
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

		// Communication ends if server sent an "end" message
		if (strcmp(recvMessage, "end") == 0) done = true;


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

void readData(int& num1, int& num2)
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	// Read the version number and discard it
	int tmp = num1 = readInt(dataFile);

	// Read the two data values
	num1 = readInt(dataFile);
	num2 = readInt(dataFile);

	dataFile.close();
}

void cleanup(SOCKET socket)
{
	closesocket(socket);
	WSACleanup();
}