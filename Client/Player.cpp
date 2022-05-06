#include "IntVector2D.h"
#include "Player.h"

Player::Player(IntVector2D position, int ID)
	: mID(ID),
	mPosition(position)
{
}

Player::Player(Player& other)
	: mID(other.mID),
	mPosition(other.mPosition)
{
}

void Player::operator=(Player& other)
{
	mID = other.mID;
	mPosition = other.mPosition;
}

Player::~Player()
{
}

int Player::GetID(void)
{
	return mID;
}

IntVector2D Player::GetPosition(void)
{
	return mPosition;
}

void Player::Move(IntVector2D moveOffset)
{
	mPosition.X += moveOffset.X;
	mPosition.Y += moveOffset.Y;
}
