#pragma once

struct UserInfo
{
	SOCKET Socket;
	SOCKADDR_IN Addr;
	int32_t ID;
	int32_t X;
	int32_t Y;
	bool IsConnected;

	UserInfo(SOCKET socket, SOCKADDR_IN addr, int32_t userID, int32_t x, int32_t y)
		: Socket(socket),
		Addr(addr),
		ID(userID),
		IsConnected(true),
		X(x),
		Y(y)
	{
	}
};