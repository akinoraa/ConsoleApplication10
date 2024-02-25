#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main()
{
	setlocale(0, "");
	system("title SERVER SIDE");

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed with error: " << iResult << "\n";
		cout << "Connection to Winsock.dll failed!\n";
		return 1;
	}

	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* result = NULL;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo failed with error: " << iResult << "\n";
		cout << "Server address and port retrieval failed!\n";
		WSACleanup();
		return 2;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		cout << "socket failed with error: " << WSAGetLastError() << "\n";
		cout << "Socket creation failed!\n";
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		cout << "bind failed with error: " << WSAGetLastError() << "\n";
		cout << "Socket binding to IP address failed!\n";
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		cout << "listen failed with error: " << WSAGetLastError() << "\n";
		cout << "Listening for client information failed. Something went wrong!\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 5;
	}

	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		cout << "accept failed with error: " << WSAGetLastError() << "\n";
		cout << "Connection to client application failed! Sadness!\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 6;
	}

	closesocket(ListenSocket);

	//////////////////////////////////////////////////////////


	printf("Server is running. Waiting for connections...\n");

	do
	{
		char receivedData[DEFAULT_BUFLEN];
		iResult = recv(ClientSocket, receivedData, DEFAULT_BUFLEN, 0);
		receivedData[iResult] = '\0';

		if (iResult > 0)
		{
			for (int i = 0; i < iResult; ++i)
			{
				if (receivedData[i] == '.')
				{
					receivedData[i] = ',';
				}
			}


			double clientNumber;
			if (sscanf_s(receivedData, "%lf", &clientNumber) == 1) {

				double responseNumber = clientNumber + 1;

				char response[DEFAULT_BUFLEN];
				snprintf(response, sizeof(response), "%.2lf", responseNumber);

				iResult = send(ClientSocket, response, strlen(response), 0);
				if (iResult == SOCKET_ERROR) {
					printf("Failed to send with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 7;
				}
			}
			else {
				printf("Incorrect number received from client: %s\n", receivedData);
			}
		}


	} while (iResult > 0);

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Shutdown failed with error: " << WSAGetLastError() << "\n";
		cout << "Oops, connection shutdown threw an error ((\n";
		closesocket(ClientSocket);
		WSACleanup();
		return 9;
	}

	closesocket(ClientSocket);
	WSACleanup();
}
