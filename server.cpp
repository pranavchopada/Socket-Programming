#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main()
{
	// Initialze winsock with WSADATA
	WSADATA wsData;
	//Creates a word value by concatenating the lower byte and higher byte
	WORD ver = MAKEWORD(2, 2);
	//WSAStartup initializes winsock and we pass word and a reference of WSADATA
	int wsOk = WSAStartup(ver, &wsData);
	//If wsOk is anything but zero it leads to an error and we end the program immediately
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return;
	}

	// We create a socket and specify that we are using IPv4 addresses, and SOCK_STREAM used to transfer bytes and commonly used in TCP
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
	//Similar to wsOK, if it returns anything but 0, it leads to an error
	if (listenSock == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		return;
	}

	// Bind the ip address and port to a socket using the structure
	sockaddr_in hint;
	//specify IPv4 addresses
	hint.sin_family = AF_INET;
	//specifies the port to listen at
	hint.sin_port = htons(54000);
	//Any ip address can be connected (no restrictions to a single ip)
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	//Bind function to bind the sockaddr_in to the socket we created
	bind(listenSock, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening 
	//SOMAAXCONN represents the max backlog value supported
	listen(listenSock, SOMAXCONN);

	// Wait for a connection
	cout << "Waiting for a Client..." << endl;
	//Initialize a sockaddr_in for the client
	sockaddr_in client;
	//Find the size of the client
	int clientSize = sizeof(client);

	//Accept the connection using the accept the function
	SOCKET clientSocket = accept(listenSock, (sockaddr*)&client, &clientSize);

	char host[NI_MAXHOST];		// Client's remote name
	char service[NI_MAXSERV];	// Port the client is connected on

	ZeroMemory(host, NI_MAXHOST); // Makes the memory 0
	ZeroMemory(service, NI_MAXSERV); //Zeros out the memory
	//Uses the getnameinfo funtion to get the name and port of the client
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		//prints the name of the client on the respective port
		cout << host << " connected on port " << service << endl;
	}

	// Close listening socket
	closesocket(listenSock);

	//count(used to determine whether the server should receive or send files)
	int count = 0;
	//max data that can be transferred is stored in buf
	char buf[4096];
	//stores the file name
	string file = "";
	//used to send the file size
	string line = "";
	int len = 0;
	//message type
	int flag = 0;
	//input by the user
	string userInput = "";
	while (true)
	{
		//client receives as default
		if (count == 0)
		{
			//clears the buffer
			ZeroMemory(buf, 4096);
			//receives from the client using recv
			int bytesReceived = recv(clientSocket, buf, 4096, 0);
			//if it receives something
			if (bytesReceived > 0)
			{
				//Echoes response to console
				cout << "CLIENT>> " << string(buf, 1, bytesReceived) << endl;
				//Checks the message type
				if ((string(buf, 0, 1)) == "1")
				{
					//chat data
					flag = 1;
				}
				else if (string(buf, 0, 1) == "2")
				{
					//file data
					flag = 2;
					//saves the name of the file
					file = string(buf, 1, bytesReceived);
				}
				else if (string(buf, 0, 1) == "3")
				{
					//error
					flag = 3;
				}
				else
				{
					//close connection
					flag = 4;
				}

			}
			//sets the count as 1 so that the server can do the next task
			count = 1;
		}
		else
		{
			//clears buffer
			ZeroMemory(buf, 4096);
			//for chat data
			if (flag == 1)
			{
				//prompts the server
				cout << "> ";
				getline(cin, userInput);
				// Make sure the user has typed in something
				if (userInput.size() > 0)
				{
					//adds the message type
					userInput = "1" + userInput;
					// Sends to the client with the size of the message
					int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);
					if (sendResult != SOCKET_ERROR)
					{
						//cout << "Sent Successfully!" << endl;
					}
					else
					{
						//if send results in an error means the client is disconnected
						cout << "Error in Connection" << endl;
						cout << "Connection Closed." << endl;
						cout << endl;
						//close the socket
						closesocket(clientSocket);
						WSACleanup();
						//call main to look for a new client
						main();
					}
				}
			}
			//file data
			else if (flag == 2)
			{
				//initializes input stream to read in binary
				ifstream infile(file, ios::binary | ios::in | ios::ate);
				//if file doesn't exist or open
				if (!infile)
				{
					//sends an error with message type 3
					userInput = "3File not found. Request again.";
					int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);
					if (sendResult != SOCKET_ERROR)
					{
						//cout << "Sent Successfully!" << endl;
					}
					else
					{
						//if send results in an error, the client is disconnected
						cout << "Error in Connection" << endl;
						cout << "Connection Closed." << endl;
						cout << endl;
						//close socket
						closesocket(clientSocket);
						WSACleanup();
						//call main and look for another client
						main();
					}
				}
				else
				{
					//if file opens, finds the size of the file
					int siz = infile.tellg();
					//goes back to the beginning of the file
					infile.clear();
					infile.seekg(0, ios::beg);
					//creates the message type 2 with the size of the file and sends it to the client
					line = "2" + to_string(siz);
					cout << "Sending the File...." << endl;
					int sendResult = send(clientSocket, line.c_str(), line.size() + 1, 0);
					if (sendResult != SOCKET_ERROR)
					{
						//cout << "Sent Successfully!" << endl;
					}
					else
					{
						//if send results in an error, the client is disconnected
						cout << "Error in Connection" << endl;
						cout << "Connection Closed." << endl;
						cout << endl;
						//close socket
						closesocket(clientSocket);
						WSACleanup();
						//call main and look for another client
						main();
					}
					int remaining_sent;
					//checks if siz is larger than the buffer size
					if (siz > 4096)
					{
						remaining_sent = 4096;
					}
					else
					{
						remaining_sent = siz;
					}
					int keep_count = siz;
					//uses a loop to send the entire file 
					for (int i = 0; i <= (siz) / 4096; i++)
					{
						//reads the file
						infile.read(buf, remaining_sent);
						//sends the max size of buffer at one time
						send(clientSocket, buf, remaining_sent, 0);
						//keeps count of the no of bits left to be sent
						keep_count = keep_count - remaining_sent;
						//initializes remaining_count accordingly
						if (keep_count > 4096)
						{
							remaining_sent = 4096;
						}
						else
						{
							remaining_sent = keep_count;
						}
					}
					//sends the file successfully
					cout << "File Sent Successfully" << endl;
					//closes the file
					infile.close();
				}
			}
			else if (flag == 4)
			{
				cout << "Closing Connection...." << endl;
				// Close the socket
				closesocket(clientSocket);
				// Cleanup winsock
				WSACleanup();
				cout << "Connection Closed." << endl;
				cout << endl;
				//call main to look for another client
				main();
			}
			count = 0;
		}
	}

	// Close the socket
	closesocket(clientSocket);

	// Cleanup winsock
	WSACleanup();

	system("pause");
}