#pragma comment(lib, "ws2_32")

#include <memory>
#include <vector>

#include <WinSock2.h>
#include <Windows.h>

#include "Message.h"
#include "UserInfo.h"

#include "Server.h"
#include "Console.h"

void PrintServerState(const Server* server);

int main(void)
{
	CSInitial();

	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	{
		Server* server = Server::GetInstanceOrNull();
		{
			if (server == nullptr)
			{
				wprintf(L"failed init server : %d\n", WSAGetLastError());
				goto WSA_CLEANUP;
			}

			if (!server->TryListen())
			{
				wprintf(L"failed listen : %d\n", server->GetLatestErrorCode());
				goto DESTROY_SERVER;
			}

			while (server->TryProcessNetworkJob())
			{
				PrintServerState(server);
			}
			wprintf(L"error code : %d\n", server->GetLatestErrorCode());
		}
	DESTROY_SERVER:
		Server::DestoryInstance();
	}
WSA_CLEANUP:
	WSACleanup();

	return 0;
}

static int start = GetTickCount();
static int frame;
static int fps;

void PrintServerState(const Server* server)
{
	int end = GetTickCount();

	++frame;

	if (1000 < end - start)
	{
		fps = frame;
		start = end;
		frame = 0;
	}
	CSClearScreen();
	CSMoveCursor(0, 0);
	wprintf(L"Connected User : %d   FPS : %d\n", server->GetConnectedUserCount(), fps);
}
