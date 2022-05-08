#pragma comment(lib, "ws2_32")

#include <memory>
#include <vector>

#include <WinSock2.h>
#include <Windows.h>

#include "Message.h"
#include "UserInfo.h"

#include "Server.h"

Server* Server::instance = nullptr;

Server::~Server()
{
	for (auto& user : mUserList)
	{
		closesocket(user->Socket);
	}
	closesocket(mListenSocket);
}

Server* Server::GetInstanceOrNull()
{
	int errorCode;

	if (instance != nullptr)
	{
		return instance;
	}
	instance = new Server();

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		return nullptr;
	}

	SOCKADDR_IN serverAddr;
	{
		ZeroMemory(&serverAddr, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(SERVER_PORT);
		serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

		errorCode = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (errorCode == SOCKET_ERROR)
		{
			goto CLOSE_SOCKET;
		}
		instance->mServerAddr = serverAddr;
	}
	{
		u_long on = 1;
		ioctlsocket(listenSocket, FIONBIO, &on);

		BOOL optval = true;
		setsockopt(listenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

		LINGER lingerOptval;
		{
			lingerOptval.l_onoff = 1;
			lingerOptval.l_linger = 0;
		}
		setsockopt(listenSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOptval, sizeof(lingerOptval));
	}

	instance->mListenSocket = listenSocket;
	instance->mUserList.reserve(DEFAULT_LIST_SIZE);
	instance->mLatestErrorCode = 0;
	instance->mBaseID = 0;

	FD_ZERO(&instance->mGlobalReadSet);

	return instance;

CLOSE_SOCKET:
	closesocket(listenSocket);

	return nullptr;
}

void Server::DestoryInstance()
{
	delete instance;
	instance = nullptr;
}

bool Server::TryListen()
{
	int errorCode = listen(mListenSocket, SOMAXCONN);
	if (errorCode == SOCKET_ERROR)
	{
		mLatestErrorCode = WSAGetLastError();
		return false;
	}
	FD_SET(mListenSocket, &mGlobalReadSet);

	return true;
}

bool Server::TryProcessNetworkJob()
{
	Message message;
	FD_SET readSet;
	timeval timeout;
	{
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
	}

	memcpy(&readSet, &mGlobalReadSet, sizeof(readSet));

	select(0, &readSet, nullptr, nullptr, &timeout);
	
	if (FD_ISSET(mListenSocket, &readSet))
	{
		SOCKET acceptedSocket;
		SOCKADDR_IN clientAddr;

		int addrSize = sizeof(clientAddr);

		acceptedSocket = accept(mListenSocket, (SOCKADDR*)&clientAddr, &addrSize);
		if (acceptedSocket == INVALID_SOCKET)
		{
			mLatestErrorCode = WSAGetLastError();
			return false;
		}
		UserInfo* acceptedUser = new UserInfo(acceptedSocket, clientAddr, mBaseID, INIT_POS_X, INIT_POS_Y);

		FD_SET(acceptedSocket, &mGlobalReadSet);
		mUserList.push_back(std::unique_ptr<UserInfo>(acceptedUser));

		SetMessageData(&message, eMessageType::AssignID, mBaseID, 0, 0);
		sendUnicast(*acceptedUser, message);

		SetMessageData(&message, eMessageType::CreateUser, mBaseID, INIT_POS_X, INIT_POS_Y);
		sendBroadCast(*acceptedUser, message);

		for (auto& userInfo : mUserList)
		{
			SetMessageData(&message, eMessageType::CreateUser, userInfo->ID, userInfo->X, userInfo->Y);
			sendUnicast(*acceptedUser, message);
		}

		++mBaseID;
	}

	for (auto& userInfo : mUserList)
	{
		if (FD_ISSET(userInfo->Socket, &readSet))
		{
			while (true)
			{
				int errorCode = recv(userInfo->Socket, (char*)&message, sizeof(message), 0);

				if (errorCode == SOCKET_ERROR || errorCode == 0)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						userInfo->IsConnected = false;
					}
					break;
				}

				userInfo->X = message.Data[2];
				userInfo->Y = message.Data[3];

				SetMessageData(&message, eMessageType::MoveUser, userInfo->ID, userInfo->X, userInfo->Y);
				sendBroadCast(*userInfo, message);
			}
		}
	}

	auto iter = mUserList.begin();

	while(iter != mUserList.end())
	{
		auto& userInfo = *iter;

		if (userInfo->IsConnected)
		{
			++iter;
		}
		else
		{
			SetMessageData(&message, eMessageType::DeleteUser, userInfo->ID, 0, 0);
			sendBroadCast(*userInfo, message);

			FD_CLR(userInfo->Socket, &mGlobalReadSet);
			closesocket(userInfo->Socket);

			iter = mUserList.erase(iter);
		}
	}

	return true;
}

int Server::GetLatestErrorCode() const
{
	return mLatestErrorCode;
}

int Server::GetConnectedUserCount() const
{
	return (int)mUserList.size();
}

void Server::sendUnicast(UserInfo& userInfo, const Message& message)
{
	int errorCode = send(userInfo.Socket, (char*)&message, sizeof(message), 0);

	if (errorCode == SOCKET_ERROR)
	{
		userInfo.IsConnected = false;
	}
}

void Server::sendBroadCast(UserInfo& excluded, const Message& message)
{
	for (auto& userInfo : mUserList)
	{
		if (userInfo->Socket != excluded.Socket)
		{
			sendUnicast(*userInfo, message);
		}
	}
}

