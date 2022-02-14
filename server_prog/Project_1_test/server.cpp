#include<iostream>
#include<fstream>
#include<WS2tcpip.h>	
#include <iomanip>
#include<string>
#define SERVER_PORT 2955
#pragma comment (lib, "ws2_32.lib")
using namespace std;


int main()
{
	int count = 1000;
	fstream filein;
	fstream output;

	ofstream ofs; 
	ofs.open("Book.txt", ofstream::out | ofstream::trunc);
	ofs.close();

	output.open("Book.txt", ios::out|ios::in);

	//initialize winsock
	WSADATA winsData;
	WORD verReq = MAKEWORD(2, 2); //to request which version we want, 2.2

	int wsOk = WSAStartup(verReq, &winsData);
	if (wsOk != 0)
	{
		cerr << "Error initializing winsock. Try again. \n";
		return 0;
	}

	//create socket
	SOCKET listening = socket(PF_INET, SOCK_STREAM, 0); //takes address family, opens tcp socket, listening is our socket
	if (listening == INVALID_SOCKET)
	{
		cerr << "Socket creation failed. Try again. \n";
		return 0;
	}
	//                     more error handling?
	
	//bind socket to an ip address and port
	sockaddr_in sin; //fill in a hint structure//indicates preferred socket/protocol 
	sin.sin_family = PF_INET; //using version 4
	sin.sin_port = htons(SERVER_PORT); //htons=host to network short
	sin.sin_addr.S_un.S_addr = INADDR_ANY; //to bind to any address

	bind(listening, (sockaddr*)&sin, sizeof(sin)); //binds to networking code, file descriptor

	//tell winsock , the socket is listening
	listen(listening, SOMAXCONN);
	//wait for a connection

	sockaddr_in client; //input address for client
	int clientSize = sizeof(client);

	SOCKET new_s = accept(listening, (sockaddr*)&client, &clientSize);
	if (new_s == INVALID_SOCKET)
	{
		cerr << "Invalid socket. Try again. \n";
		return 0;	

	}
	

	char host[NI_MAXHOST]; //clients remote name
	char service[NI_MAXHOST]; // port the client connects to, service buffer

	memset(host, 0, NI_MAXHOST);  //setting to zero
	memset(service, 0, NI_MAXHOST);

	    // check if we can get name info, if not rely on what we have
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cerr << host << "Connected on port" << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cerr << host << " connected on port" << ntohs(client.sin_port) << endl;
	}
	//close listening socket
	closesocket(listening);

	//accept message from client

	char buf[4096]; //mem for buf message

	while (true) 
	{
		memset(buf, 0, 4096);

		//wait for client to send data
		int bytesRec = recv(new_s, buf, 4096, 0);
		if (bytesRec == SOCKET_ERROR)
		{
			cerr << "Error recieving. Try again. \n";
			break;
		}
		if (bytesRec == 0)
		{
			cerr << "Disconnected from client. \n"; //when client disconnects, or quits
			break;
		}

		// address book editing
		
		//command word being sent (add,delete,list,shutdown,quit)
		string commWord;
		//command message being sent
		string commMsg = string(buf, 0, bytesRec);
		//cerr << commMsg << endl;
		for (int i = 0; i < commMsg.size() - 1; i++) 
		{
			if (commMsg.at(i) == ' ')
			{
				commWord = commMsg.substr(0, i); //command
				commMsg = commMsg.substr(i + 1); //second part of message after command
				break;
			}
		}
		if (commWord == "ADD")
		{
			//puts output to end of file
			output.seekg(output.tellg(), ios::end);
			//output count 
			output  <<++count << " " << left << setw(25) << commMsg  << "\n";
		}
		else if (commWord == "DELETE")
		{
			string book;
			string blank = "                             "; //replaces deleted message
			output.clear();
			//moves pointer back to beginning of file
			output.seekg(output.tellg(), ios::beg);
			output.seekg(0, ios::beg);
			output.clear();
			output >> book;
			int pos = output.tellg();

			while (output.good())
			{

				if (commMsg == book)
				{

					output.seekg(output.tellg(), ios::beg); //resets to where we currently are
					output.seekg(pos);
					
				
					output << blank << endl; //deletes
					break;
				}
			
				output >> book;
				pos = output.tellg();

				for (int i = 0; i < book.size() - 1; ++i) 
				{
					if (isalnum(book.at(i))) // if character is true
					{ 
						book=book.substr(i);
						break;
					}
				}
				

			}

			if (!output.good())
			{
				cout << "Invalid address, try again." << endl;
				output.clear(); //sets bit to good
				output.seekg(output.tellg(), ios::beg);

			}

			// search for address
			output.seekg(32);
			// go back an int bit
		}
		else if (commMsg == "LIST")
		{
			string book;
			output.clear();
			//moves pointer back to beginning of file
			output.seekg(output.tellg(),ios::beg);
			output.seekg(0, ios::beg);
			output.clear();
			getline(output, book);
			
			//while good, keep printing
			while (output.good())
			{
				cout << book << endl;
				getline(output, book);
			}
			cout << book << endl;
			//clears error bit in output
			output.clear();

		}
		else if (commMsg == "QUIT")
		{
			cout << "Client now closed." << endl;
			//WSACleanup();
			//closes just client

		}
		else if (commWord == "SHUTDOWN") //comword orig
		{
			closesocket(new_s);
			exit(0); //closes both and server
			break;
			
		}
		else 
		{
			//error
			cout << "403 Message Format Error, try again." << endl;
		}


		
		//echoes message back to client

		send(new_s, buf, bytesRec + 1, 0);
		
	}



	//close socket

	closesocket(new_s);
	//shutdown winsock
	WSACleanup();

	return 0;
}