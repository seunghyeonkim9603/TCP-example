#pragma warning(disable : 4996 6031)
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <Windows.h>

#include <assert.h>
#include <memory>
#include <stdio.h>
#include <vector>

#include "Console.h"
#include "Message.h"
#include "Player.h"

#include "ApplicationManager.h"
#include "Visualizer.h"

#define SERVER_PORT (3000)
#define BUFFER_SIZE (256)

int numPacket = 0;

int main(void)
{
	CSInitial();
	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	{
		ApplicationManager manager;
		SOCKET clientSocket;
		int errorCode;

		timeval timeout;
		{
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;
		}

		SOCKADDR_IN serverAddr;
		{
			char IPbuffer[BUFFER_SIZE];

			wprintf(L"Connect to :");
			fgets(IPbuffer, BUFFER_SIZE, stdin);

			ZeroMemory(&serverAddr, sizeof(serverAddr));
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(SERVER_PORT);
			serverAddr.sin_addr.S_un.S_addr = inet_addr(IPbuffer);
		}

		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (clientSocket == INVALID_SOCKET)
		{
			wprintf(L"socket() error : %d\n", WSAGetLastError());
			goto WSA_CLEANUP;
		}

		errorCode = connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

		if (errorCode == SOCKET_ERROR)
		{
			wprintf(L"connect() error : %d\n", WSAGetLastError());
			goto CLOSE_SOCKET;
		}

		{
			BOOL optval = true;
			u_long on = 1;
			setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));
			ioctlsocket(clientSocket, FIONBIO, &on);
		}

		while (true)
		{
			Message message;
			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(clientSocket, &readSet);

			manager.Update(&message);

			if (message.Data[0] == (int32_t)eMessageType::MoveUser)
			{
				errorCode = send(clientSocket, (char*)&message, sizeof(message), 0);
				if (errorCode == SOCKET_ERROR)
				{
					wprintf(L"send() error : %d\n", WSAGetLastError());
					goto CLOSE_SOCKET;
				}
			}
			
			errorCode = select(0, &readSet, nullptr, nullptr, &timeout);
			if (errorCode == SOCKET_ERROR)
			{
				wprintf(L"select() error %d\n", WSAGetLastError());
				goto CLOSE_SOCKET;
			}

			if (FD_ISSET(clientSocket, &readSet))
			{
				while (true)
				{
					errorCode = recv(clientSocket, (char*)&message, sizeof(message), 0);
					if (errorCode == SOCKET_ERROR)
					{
						if (WSAGetLastError() == WSAEWOULDBLOCK)
						{
							break;
						}
						goto CLOSE_SOCKET;
					}
					manager.ProcessMessage(message);
					numPacket++;
				}
			}
			CSMoveCursor(0, 1);
			printf("num Packet : %d", numPacket);
			Visualize(manager.GetPlayerList());
		}
	CLOSE_SOCKET:
		closesocket(clientSocket);
	}
WSA_CLEANUP:
	WSACleanup();

	return 0;
}