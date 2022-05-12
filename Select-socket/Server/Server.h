#pragma once

class Server final
{
public:
	Server(Server& other) = delete;
	void operator=(Server& other) = delete;
	~Server();

	static Server* GetInstanceOrNull();
	static void DestoryInstance();

	bool TryListen();
	bool TryProcessNetworkJob();

	int GetLatestErrorCode() const;
	int GetConnectedUserCount() const;

private:
	Server() = default;

	void sendUnicast(UserInfo& userInfo, const Message& message);
	void sendBroadCast(UserInfo& excluded, const Message& message);

private:
	enum
	{
		SERVER_PORT = 3000,
		DEFAULT_LIST_SIZE = 256,
		INIT_POS_X = 40,
		INIT_POS_Y = 11
	};

	static Server* instance;

	SOCKET mListenSocket;
	SOCKADDR_IN mServerAddr;

	std::vector<std::unique_ptr<UserInfo>> mUserList;

	FD_SET mGlobalReadSet;

	int32_t mBaseID;
	int mLatestErrorCode;
};