#pragma once;

class Player final
{
public:
	Player() = delete;
	Player(int x, int y, int ID);
	Player(Player& other);
	void operator=(Player& other);
	bool operator==(const Player& other) const;
	~Player();

	int GetID() const;
	int GetX() const;
	int GetY() const;
	void Move(int toX, int toY);

private:
	int mID;
	int mX;
	int mY;
};