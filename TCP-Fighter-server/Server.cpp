#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <Windows.h>

#include <unordered_map>

#include "Action.h"
#include "Direction.h"
#include "Point.h"
#include "BoundingRect.h"
#include "RingBuffer.h"
#include "User.h"
#include "Quadrant.h"
#include "Packet.h"
#include "Server.h"

Server* Server::instance = nullptr;

Server::Server(const SOCKET listenSocket, const BoundingRect& rect)
	: mListenSocket(listenSocket),
	mIDBase(0),
	mLatestErrorCode(0),
	mUsers(517),
	mQuadrant(rect)
{
}

Server::~Server()
{
	for (auto userIter : mUsers)
	{
		closesocket(userIter.first);
		delete userIter.second;
	}
	closesocket(mListenSocket);
}

Server* Server::GetInstanceOrNull()
{
	if (instance != nullptr)
	{
		return instance;
	}

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
		serverAddr.sin_addr.S_un.S_addr = SERVER_IP;

		int bindErrorCode = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (bindErrorCode == SOCKET_ERROR)
		{
			closesocket(listenSocket);
			return nullptr;
		}
	}
	u_long on = 1;
	ioctlsocket(listenSocket, FIONBIO, &on);

	LINGER lingerOptval;
	{
		lingerOptval.l_onoff = 1;
		lingerOptval.l_linger = 0;
	}
	setsockopt(listenSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOptval, sizeof(lingerOptval));

	BoundingRect rect({ 0, 0 }, SCREEN_WIDTH, SCREEN_HEIGHT);

	instance = new Server(listenSocket, rect);

	return instance;
}

void Server::DestroyInstance()
{
	if (instance != nullptr)
	{
		delete instance;
	}
	instance = nullptr;
}

bool Server::TryListen()
{
	int listenErrorCode = listen(mListenSocket, SOMAXCONN);
	if (listenErrorCode == SOCKET_ERROR)
	{
		mLatestErrorCode = WSAGetLastError();
		return false;
	}
	return true;
}

bool Server::TryProcessNetworkJob()
{
	FD_SET readSet;
	FD_SET writeSet;
	timeval timeout = { 0, 0 };

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_SET(mListenSocket, &readSet);

	for (auto iter : mUsers)
	{
		SOCKET userSocket = iter.second->Socket;

		FD_SET(userSocket, &readSet);
		FD_SET(userSocket, &writeSet);
	}

	int selectErrorCode = select(0, &readSet, &writeSet, nullptr, &timeout);

	if (selectErrorCode == SOCKET_ERROR)
	{
		mLatestErrorCode = WSAGetLastError();
		return false;
	}
	
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

		User* newUser = new User();
		{
			newUser->Socket = acceptedSocket;
			newUser->ID = mIDBase;
			newUser->Hp = INIT_HP;
			newUser->Position.X = INIT_POS_X;
			newUser->Position.Y = INIT_POS_Y;
			newUser->Action = eAction::Stand;
			newUser->Direction = eDirection::Left;
			newUser->bIsConnected = true;
			newUser->Port = ntohs(clientAddr.sin_port);
			newUser->IP = ntohl(clientAddr.sin_addr.S_un.S_addr);
		}

		mQuadrant.TryInsert(newUser);
		mUsers.insert({ newUser->ID, newUser });

		SCCreateMyCharacterPacket createMyCharacterPacket;
		SCCreateOtherCharacterPacket createOtherCharacterPacket;
		{
			createMyCharacterPacket
				.SetData(newUser->ID, (__int8)newUser->Direction, newUser->Position.X, newUser->Position.Y, newUser->Hp);

			createOtherCharacterPacket
				.SetData(newUser->ID, (__int8)newUser->Direction, newUser->Position.X, newUser->Position.Y, newUser->Hp);
		}

		sendBroadcast(newUser->ID, (char*)&createOtherCharacterPacket, sizeof(createOtherCharacterPacket));
		sendUnicast(newUser->ID, (char*)&createMyCharacterPacket, sizeof(createMyCharacterPacket));

		for (auto iter : mUsers)
		{
			User* user = iter.second;

			if (user->ID == newUser->ID)
			{
				continue;
			}

			createOtherCharacterPacket.SetData(user->ID, (__int8)user->Direction, user->Position.X, user->Position.Y, user->Hp); // ÃÊ±âÈ­
			sendUnicast(newUser->ID, (char*)&createOtherCharacterPacket, sizeof(createOtherCharacterPacket));

			if (user->Action == eAction::Move)
			{
				SCMoveStartPacket moveStartPacket;

				moveStartPacket.SetData(user->ID, (__int8)user->Direction, user->Position.X, user->Position.Y);
				sendUnicast(newUser->ID, (char*)&moveStartPacket, sizeof(moveStartPacket));
			}
		}
		mIDBase++;
	}

	for (auto iter : mUsers)
	{
		User* user = iter.second;
		if (FD_ISSET(user->Socket, &readSet) && user->bIsConnected)
		{
			RingBuffer& receiveBuffer = user->ReceiveBuffer;

			int numReceived = recv(user->Socket, receiveBuffer.GetRear(), receiveBuffer.GetDirectEnqueueableSize(), 0);
			
			if (numReceived == SOCKET_ERROR || numReceived == 0)
			{
				user->bIsConnected = false;
				continue;
			}
			receiveBuffer.MoveRear(numReceived);

			while (!receiveBuffer.IsEmpty() && user->bIsConnected)
			{
				PacketHeader header;
				char payload[256];

				if (!receiveBuffer.TryPeek((char*)&header, sizeof(header)))
				{
					break;
				}
				if (receiveBuffer.GetSize() < header.Size + sizeof(header))
				{
					break;
				}
				receiveBuffer.MoveFront(sizeof(header));

				if (!receiveBuffer.TryDequeue(payload, header.Size))
				{
					break;
				}

				processReceivedPacket(*user, header, payload);
			}
		}
		if (FD_ISSET(user->Socket, &writeSet) && user->bIsConnected)
		{
			RingBuffer& sendBuffer = user->SendBuffer;

			int numSent = send(user->Socket, sendBuffer.GetFront(), sendBuffer.GetDirectDequeueableSize(), 0);

			if (numSent == SOCKET_ERROR)
			{
				user->bIsConnected = false;
				continue;
			}
			sendBuffer.MoveFront(numSent);
		}
	}

	auto iter = mUsers.begin();

	while (iter != mUsers.end())
	{
		User* user = iter->second;

		if (user->bIsConnected)
		{
			++iter;
		}
		else
		{
			SCDeleteCharacterPacket deletePacket;

			deletePacket.SetData(user->ID);
			sendUnicast(user->ID, (char*)&deletePacket, sizeof(deletePacket));
			sendBroadcast(user->ID, (char*)&deletePacket, sizeof(deletePacket));

			closesocket(user->Socket);
			mQuadrant.TryDelete(user->ID);
			delete user;

			iter = mUsers.erase(iter);
		}
	}

	return true;
}

void Server::Update()
{
	for (auto iter : mUsers)
	{
		User* user = iter.second;

		if (user->Action == eAction::Move)
		{
			int x = user->Position.X;
			int y = user->Position.Y;

			switch (user->Direction)
			{
			case eDirection::Left:
				x -= MOVE_OFFSET_X;
				break;
			case eDirection::LeftUp:
				x -= MOVE_OFFSET_X;
				y -= MOVE_OFFSET_Y;
				break;
			case eDirection::Up:
				y -= MOVE_OFFSET_Y;
				break;
			case eDirection::RightUp:
				x += MOVE_OFFSET_X;
				y -= MOVE_OFFSET_Y;
				break;
			case eDirection::Right:
				x += MOVE_OFFSET_X;
				break;
			case eDirection::RightDown:
				x += MOVE_OFFSET_X;
				y += MOVE_OFFSET_Y;
				break;
			case eDirection::Down:
				y += MOVE_OFFSET_Y;
				break;
			case eDirection::LeftDown:
				x -= MOVE_OFFSET_X;
				y += MOVE_OFFSET_Y;
				break;
			default:
				break;
			}
			if (x <= RANGE_MOVE_LEFT || RANGE_MOVE_RIGHT <= x
				|| y <= RANGE_MOVE_TOP || RANGE_MOVE_BOTTOM <= y)
			{
				continue;
			}
			user->Position.X = x;
			user->Position.Y = y;

			mQuadrant.TryRelocate(user->ID);
			printf("Move   ID : [%d]   toX : [%d]   toY : [%d]\n", user->ID, user->Position.X, user->Position.Y);
		}
	}
}

void Server::sendUnicast(const int ID, const char* packet, int length)
{
	auto iter = mUsers.find(ID);

	User* user = iter->second;
	RingBuffer& sendBuffer = user->SendBuffer;

	if (!sendBuffer.TryEnqueue(packet, length))
	{
		user->bIsConnected = false;
	}
}

void Server::sendBroadcast(const int excludedID, const char* packet, int length)
{
	for (auto iter : mUsers)
	{
		User* user = iter.second;
		if (user->ID != excludedID && user->bIsConnected)
		{
			RingBuffer& sendBuffer = user->SendBuffer;

			if (!sendBuffer.TryEnqueue(packet, length))
			{
				user->bIsConnected = false;
			}
		}
	}
}

void Server::processReceivedPacket(User& sentUser, const PacketHeader& header, const char* payload)
{
	if (header.Code != (char)PACKET_CODE)
	{
		sentUser.bIsConnected = false;
		return;
	}
	ePacketType type = (ePacketType)header.Type;

	switch (type)
	{
	case ePacketType::CSMoveStart:
		processMoveStartPacket(sentUser, payload);
		break;
	case ePacketType::CSMoveStop:
		processMoveStopPacket(sentUser, payload);
		break;
	case ePacketType::CSAttack1:
		processAttack1Packet(sentUser, payload);
		break;
	case ePacketType::CSAttack2:
		processAttack2Packet(sentUser, payload);
		break;
	case ePacketType::CSAttack3:
		processAttack3Packet(sentUser, payload);
		break;
	default:
		sentUser.bIsConnected = false;
		break;
	}
}

void Server::processMoveStartPacket(User& user, const char* payload)
{
	CSMoveStartPacket* receivedPacket;

	receivedPacket = (CSMoveStartPacket*)payload;

	user.Action = eAction::Move;
	user.Position.X = receivedPacket->X;
	user.Position.Y = receivedPacket->Y;
	user.Direction = (eDirection)receivedPacket->Direction;

	SCMoveStartPacket sendPacket;

	sendPacket.SetData(user.ID, (__int8)user.Direction, user.Position.X, user.Position.Y);
	sendBroadcast(user.ID, (char*)&sendPacket, sizeof(sendPacket));

	printf("Received MoveStartPacket    ID : [%d]   X : [%d]   Y : [%d]\n", user.ID, user.Position.X, user.Position.Y);
}

void Server::processMoveStopPacket(User& user, const char* payload)
{
	CSMoveStopPacket* receivedPacket;

	receivedPacket = (CSMoveStopPacket*)payload;

	user.Action = eAction::Stand;
	user.Position.X = receivedPacket->X;
	user.Position.Y = receivedPacket->Y;
	user.Direction = (eDirection)receivedPacket->Direction;

	SCMoveStopPacket sendPacket;

	sendPacket.SetData(user.ID, (__int8)user.Direction, user.Position.X, user.Position.Y);
	sendBroadcast(user.ID, (char*)&sendPacket, sizeof(sendPacket));

	mQuadrant.TryRelocate(user.ID);

	printf("Received MoveStopPacket    ID : [%d]   X : [%d]   Y : [%d]\n", user.ID, user.Position.X, user.Position.Y);
}

void Server::processAttack1Packet(User& user, const char* payload)
{
	CSAttack1Packet* receivedPacket;

	receivedPacket = (CSAttack1Packet*)payload;

	user.Position.X = receivedPacket->X;
	user.Position.Y = receivedPacket->Y;
	user.Direction = (eDirection)receivedPacket->Direction;

	SCAttack1Packet sendPacket;

	sendPacket.SetData(user.ID, (__int8)user.Direction, user.Position.X, user.Position.Y);
	sendBroadcast(user.ID, (char*)&sendPacket, sizeof(sendPacket));

	processAttackPacket(user, ATTACK_1_RANGE_X, ATTACK_1_RANGE_Y, ATTACK_1_DAMAGE);

	printf("Received Attack1Packet    ID : [%d]   X : [%d]   Y : [%d]\n", user.ID, user.Position.X, user.Position.Y);
}

void Server::processAttack2Packet(User& user, const char* payload)
{
	CSAttack2Packet* receivedPacket;

	receivedPacket = (CSAttack2Packet*)payload;

	user.Position.X = receivedPacket->X;
	user.Position.Y = receivedPacket->Y;
	user.Direction = (eDirection)receivedPacket->Direction;

	SCAttack2Packet sendPacket;

	sendPacket.SetData(user.ID, (__int8)user.Direction, user.Position.X, user.Position.Y);
	sendBroadcast(user.ID, (char*)&sendPacket, sizeof(sendPacket));
	
	processAttackPacket(user, ATTACK_2_RANGE_X, ATTACK_2_RANGE_Y, ATTACK_2_DAMAGE);

	printf("Received Attack2Packet    ID : [%d]   X : [%d]   Y : [%d]\n", user.ID, user.Position.X, user.Position.Y);
}

void Server::processAttack3Packet(User& user, const char* payload)
{
	CSAttack3Packet* receivedPacket;

	receivedPacket = (CSAttack3Packet*)payload;

	user.Position.X = receivedPacket->X;
	user.Position.Y = receivedPacket->Y;
	user.Direction = (eDirection)receivedPacket->Direction;

	SCAttack3Packet sendPacket;

	sendPacket.SetData(user.ID, (__int8)user.Direction, user.Position.X, user.Position.Y);
	sendBroadcast(user.ID, (char*)&sendPacket, sizeof(sendPacket));

	processAttackPacket(user, ATTACK_3_RANGE_X, ATTACK_3_RANGE_Y, ATTACK_3_DAMAGE);

	printf("Received Attack3Packet    ID : [%d]   X : [%d]   Y : [%d]\n", user.ID, user.Position.X, user.Position.Y);
}

void Server::processAttackPacket(User& attacker, int xRange, int yRange, int damage)
{
	Point topLeft;

	int x = attacker.Position.X;
	int y = attacker.Position.Y;
	int width = 0;
	int height = 0;

	topLeft.Y = y;

	if (attacker.Direction == eDirection::Left)
	{
		if (x - xRange < 0)
		{
			width = x;
			topLeft.X = 0;
		}
		else
		{
			width = xRange;
			topLeft.X = x - xRange;
		}
	}
	else if (attacker.Direction == eDirection::Right)
	{
		if (SCREEN_WIDTH < x + xRange)
		{
			width = SCREEN_WIDTH - x;
		}
		else
		{
			width = xRange;
		}
		topLeft.X = x;
	}
	else
	{
		attacker.bIsConnected = false;
		return;
	}

	if (SCREEN_HEIGHT < y + yRange)
	{
		height = SCREEN_HEIGHT - y;
	}
	else
	{
		height = yRange;
	}
	topLeft.Y = y;

	BoundingRect hitBox(topLeft, width, height);

	std::unordered_map<int, User*> expected = mQuadrant.GetUsers(hitBox);

	for (auto iter : expected)
	{
		User* target = iter.second;

		if (hitBox.Contains(target->Position))
		{
			if (target->ID == attacker.ID)
			{
				continue;
			}
			target->Hp -= damage;

			SCDamagePacket damagePacket;

			damagePacket.SetData(attacker.ID, target->ID, target->Hp);
			sendUnicast(attacker.ID, (char*)&damagePacket, sizeof(damagePacket));
			sendBroadcast(attacker.ID, (char*)&damagePacket, sizeof(damagePacket));

			if (target->Hp <= 0)
			{
				target->bIsConnected = false;
			}
		}
	}
}
