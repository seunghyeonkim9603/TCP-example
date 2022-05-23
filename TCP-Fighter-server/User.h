#pragma once

struct User
{
	SOCKET Socket;

	int ID;
	int IP;
	short Port;
	Point Position;
	eAction Action;
	eDirection Direction;
	signed char Hp;

	RingBuffer SendBuffer;
	RingBuffer ReceiveBuffer;

	bool bIsConnected;
};