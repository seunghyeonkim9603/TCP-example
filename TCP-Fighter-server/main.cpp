#pragma warning(disable : 28159 6031)
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm.lib")

#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include <vector>
#include <unordered_map>

#include "Action.h"
#include "Direction.h"
#include "Point.h"
#include "RingBuffer.h"
#include "User.h"
#include "BoundingRect.h"
#include "Quadrant.h"
#include "Packet.h"
#include "Server.h"

int main(void)
{
	WSADATA wsaData;

	timeBeginPeriod(1);

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	{
		Server* server = Server::GetInstanceOrNull();
		{
			if (!server->TryListen())
			{
				printf("listen error : %d\n", WSAGetLastError());
				assert(false);
			};
			while (true)
			{
				int start = GetTickCount();
				if (!server->TryProcessNetworkJob())
				{
					printf("process network job error : %d\n", WSAGetLastError());
					break;
				}

				server->Update();
				
				int sleepTime = 20 - (GetTickCount() - start);

				if (0 < sleepTime)
				{
					Sleep(sleepTime);
				}
			}
		}
		Server::DestroyInstance();
	}
	WSACleanup();

	return 0;
}