#include "Player.h"

Player::Player(int x, int y, int ID)
	: mID(ID),
	mX(x),
	mY(y)
{
}

Player::Player(Player& other)
	: mID(other.mID),
	mX(other.mX),
	mY(other.mY)
{
}

void Player::operator=(Player& other)
{
	mID = other.mID;
	mX = other.mX;
	mY = other.mY;
}

bool Player::operator==(const Player& other) const
{
	return mID == other.mID;
}

Player::~Player()
{
}

int Player::GetID() const
{
	return mID;
}

int Player::GetX() const
{
	return mX;
}

int Player::GetY() const
{
	return mY;
}

void Player::Move(int toX, int toY)
{
	mX = toX;
	mY = toY;
}
