#include<iostream>
#include<WS2tcpip.h>	
#include<string>
#define SERVER_PORT 2955

#pragma comment (lib, "ws2_32.lib")

using namespace std;

int main()
{
	int count = 1000;
	string ip = "127.0.0.1"; //server ip
	//int SERVER_PORT = 2955; // listening port number

	//intialize Winsock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int iRes = WSAStartup(ver, &data);
	if (iRes != 0)
	{
		cerr << "Unable to start Winsock, Error:" << iRes << endl;
		return 0;
	}
	//create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Unable to create socket, Error:" << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}

	//fill in hint structure
	sockaddr_in sin; //buffer
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, ip.c_str(), &sin.sin_addr);
	//connect to a server
	int connRes = connect(sock, (sockaddr*)&sin, sizeof(sin));
	if (connRes == SOCKET_ERROR)
	{
		cerr << "Unable to connect to server, Error:" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 0;

	}
	//loop to send and recieve datas

	char buf[4096];
	string userInput;

	//user input
	do {
		cout << "client: ";
		getline(cin, userInput);

		if (userInput.size() > 0)
		{
			//sending message once verified
			int mesRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
			if (mesRes != SOCKET_ERROR)
			{
				int bytesRec = recv(sock, buf, 4096, 0);
				if (bytesRec > 0)
				{
					//if add,del,shutdown,quit,list

					string commWord = string(buf, 0, bytesRec);
					string commMsg = string(buf, 0, bytesRec);

					for (int i = 0; i < commMsg.size() - 1; i++)
					{
						if (commMsg.at(i) == ' ')
						{
							commWord = commMsg.substr(0, i);
							commMsg = commMsg.substr(i + 1); //second part of message after command
							break;
						}
					}
					//cout << "Server->" << string(buf, 0, bytesRec) << endl; //testing purposes, original

					if (commWord == "ADD")
					{
						cout << "200 OK" << endl << "The new record is: " << ++count << endl;
					}

					else if (commWord == "QUIT")
					{
						
						//shutdown
						WSACleanup();
					}
					else if (commWord == "SHUTDOWN")
					{
						closesocket(sock);
						exit(0);
					}
				}
			}
		}

	}while (userInput.size() > 0);
	//shut down
	closesocket(sock);
	WSACleanup(); //shuts down winsock
	return 0;
	
}