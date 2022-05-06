#pragma once;

class Player final
{
public:
	Player() = delete;
	Player(IntVector2D position, int ID);
	Player(Player& other);
	void operator=(Player& other);
	~Player();

public:
	int GetID(void);
	IntVector2D GetPosition(void);
	void Move(IntVector2D moveOffset);

private:
	int mID;
	IntVector2D mPosition;
};