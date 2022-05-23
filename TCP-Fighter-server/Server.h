#pragma once

class Server final
{
public:
	Server() = delete;
	~Server();
	Server(const Server& other) = delete;
	Server& operator=(const Server& other) = delete;

	static Server* GetInstanceOrNull();
	static void DestroyInstance();

	void Update();
	bool TryListen();
	bool TryProcessNetworkJob();

private:
	Server(const SOCKET listenSocket, const BoundingRect& rect);

	void sendUnicast(const int ID, const char* packet, int size);
	void sendBroadcast(const int excludedID, const char* packet, int size);
	void processReceivedPacket(User& sentUser, const PacketHeader& header, const char* payload);

	void processMoveStartPacket(User& user, const char* payload);
	void processMoveStopPacket(User& user, const char* payload);
	void processAttack1Packet(User& user, const char* payload);
	void processAttack2Packet(User& user, const char* payload);
	void processAttack3Packet(User& user, const char* payload);
	void processAttackPacket(User& user, int xRange, int yRange, int damage);

private:
	enum
	{
		SERVER_IP = INADDR_ANY,
		SERVER_PORT = 5000,
		MAX_USERS = 256,

		INIT_POS_X = 200,
		INIT_POS_Y = 200,
		INIT_HP = 100,

		SCREEN_WIDTH = 640,
		SCREEN_HEIGHT = 480,

		ATTACK_1_DAMAGE = 1,
		ATTACK_2_DAMAGE = 2,
		ATTACK_3_DAMAGE = 3,
		ATTACK_1_RANGE_X = 80,
		ATTACK_2_RANGE_X = 90,
		ATTACK_3_RANGE_X = 100,
		ATTACK_1_RANGE_Y = 10,
		ATTACK_2_RANGE_Y = 10,
		ATTACK_3_RANGE_Y = 20,

		RANGE_MOVE_TOP = 50,
		RANGE_MOVE_LEFT = 10,
		RANGE_MOVE_RIGHT = 630,
		RANGE_MOVE_BOTTOM = 470,

		MOVE_OFFSET_X = 3,
		MOVE_OFFSET_Y = 2
	};

	static Server* instance;

	SOCKET mListenSocket;

	std::unordered_map<int, User*> mUsers;
	Quadrant mQuadrant;
	int mIDBase = 0;
	int mLatestErrorCode = 0;
};