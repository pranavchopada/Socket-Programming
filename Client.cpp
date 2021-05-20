#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <stdlib.h> 
#include <fstream>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

void main()
{
	//Stores IP in a string
	string ipAddress = "";
	//prompts for an IP address. It can connect to any device on the same network.
	cout << "Enter the IP Address of the Server you want to connect to: ";
	getline(cin, ipAddress);

	// Listening port same as the server
	int port = 54000;

	// Initialize WinSock
	WSAData data;
	//Creates a word value by concatenating the lower byte and higher byte
	WORD ver = MAKEWORD(2, 2);
	//WSAStartup initializes winsock and we pass word and a reference of WSADATA
	int wsResult = WSAStartup(ver, &data);
	//If wsResult is anything but zero it leads to an error and we end the program immediately
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return;
	}

	// We create a socket and specify that we are using IPv4 addresses, and SOCK_STREAM used to transfer bytes and commonly used in TCP
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	//Similar to wsResult, if it returns anything but 0, it leads to an error
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// Bind the ip address and port to a socket using the structure
	sockaddr_in hints;
	//specify IPv4 addresses
	hints.sin_family = AF_INET;
	//Any ip address can be connected (no restrictions to a single ip)
	hints.sin_addr.s_addr = htonl(INADDR_ANY);
	//specifies the port to listen at
	hints.sin_port = htons(port);

	//converts IPv4 to binary and binds it with the structure
	inet_pton(AF_INET, ipAddress.c_str(), &hints.sin_addr);

	// Connect to server
	int connResult = connect(sock, (sockaddr*)&hints, sizeof(hints));
	if (connResult == SOCKET_ERROR)
	{
		//can't connect and closes the socket and goes back to main
		cout << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		//close socket
		closesocket(sock);
		WSACleanup();
		main();
	}

	//Tells that the server is connected at the IPaddress
	cout << "Connected to Server at " << ipAddress << endl;

	//Lists the menu out with all Operations
	cout << "\nMenu:" << endl << "Default: send chat message" << endl << "File Transfer- request file" << endl << "End Connection - close connection" << endl << "Exit - exit\n" << endl;


	//count(used to determine whether the server should receive or send files)
	int count = 1;
	//max data that can be transferred is stored in buf
	char buf[4096];
	//input by the user
	string userInput;
	//message type
	int flag = 0;
	//total bits of a file received
	int total = 0;
	//FileName of output
	string FileName = "";
	do
	{
		if (count == 1)
		{

			cout << "> ";
			// Prompt the user for some text
			getline(cin, userInput);
			// Make sure the user has typed in something
			if (userInput.size() > 0)
			{
				// Send the text
				if (userInput == "request file")
				{
					cout << "Enter the file name: " << endl;
					cout << ">";
					// Prompt the user for some text
					getline(cin, userInput);
					//Save the name of file requested
					FileName = "Output_" + userInput;
					//Add the message type
					userInput = "2" + userInput;
					//send the message type, file name and the size of the message
					int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
					if (sendResult != SOCKET_ERROR)
					{
						//File request successful
						cout << "File requested" << endl;
					}
					else
					{
						//Server connection lost
						cout << "Error in Connection" << endl;
						cout << "Connection Closed" << endl;
						cout << endl;
						//Close Socket
						closesocket(sock);
						WSACleanup();
						main();
					}
				}
				else if (userInput == "close connection" || userInput == "exit")
				{
					string check = userInput;
					//Add the message type
					userInput = "4" + userInput;
					//send it with the file size
					int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
					if (sendResult != SOCKET_ERROR)
					{
						//Request to close connection successful
					}
					else
					{
						//Server error in connection
						cout << "Error in Connection" << endl;
						cout << "Connection Closed" << endl;
						cout << endl;
					}
					cout << "Connection Closed" << endl;
					cout << endl;
					closesocket(sock);
					WSACleanup();
					if (check == "exit")
					{
						break;
					}
					else
					{
						//calls the main to connect to another IP
						main();
					}
				}
				else
				{
					//Chat data
					userInput = "1" + userInput;
					//send the header
					int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
					if (sendResult != SOCKET_ERROR)
					{
						//Successfully Sent
						//cout << "Sent Successfully" << endl;
					}
					else
					{
						//Server Error in Connection
						cout << "Error in Connection" << endl;
						cout << "Connection Closed" << endl;
						cout << endl;
						closesocket(sock);
						WSACleanup();
						//calls main to connect to another IP
						main();
					}
				}
			}
			count = 0;
		}
		else
		{
			//clear meemory to receive
			ZeroMemory(buf, 4096);
			int bytesReceived = recv(sock, buf, 4096, 0);
			if (bytesReceived > 0)
			{
				//message type
				if ((string(buf, 0, 1)) == "1")
				{
					//chat
					flag = 1;
				}
				else if (string(buf, 0, 1) == "2")
				{
					//file
					flag = 2;
					//save the size of the file
					total = stoi(string(buf, 1, bytesReceived));
				}
				else if (string(buf, 0, 1) == "3")
				{
					//error
					flag = 3;
				}
				//file receive
				if (flag == 2)
				{
					//intializes a ofstream object
					ofstream myFile(FileName, ios::out | ios::binary);
					ZeroMemory(buf, 4096);
					int totalReceived = 0;
					//uses a loop t receive all the contents of the file
					for (int i = 0; i <= (total) / 4096; i++)
					{
						//checks the byte received
						int bytesReceived = recv(sock, buf, 4096, 0);
						//writes to the file
						myFile.write(buf, 4096);
						//calculates the total bytes received
						totalReceived += bytesReceived;
					}
					//closes the file
					myFile.close();
					//if equal
					if (total == totalReceived)
					{
						//success
						cout << "File received successfully" << endl;
					}
					else
					{
						//fails
						cout << "Error in File Received. Request Again" << endl;
					}
				}
				// Echo response to console
				else
				{
					//for chat data and error messages
					cout << "SERVER>> " << string(buf, 1, bytesReceived) << endl;
				}
			}
			count = 1;
		}

	} while (userInput.size() > 0);

	//close down everything
	closesocket(sock);
	WSACleanup();
}