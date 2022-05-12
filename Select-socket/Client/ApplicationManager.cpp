#pragma warning (disable : 26495)

#include <Windows.h>
#include <memory>
#include <vector>

#include "Console.h"
#include "Message.h"
#include "Player.h"

#include "ApplicationManager.h"

#define DEFAULT_VECTOR_SIZE (256)

ApplicationManager::ApplicationManager()
	: mUserID(INT_MIN)
{
	mPlayerList.reserve(DEFAULT_VECTOR_SIZE);
}

ApplicationManager::~ApplicationManager()
{
}


void ApplicationManager::Update(Message* outMessage)
{
	MoveUserMessage* moveMessage = (MoveUserMessage*)outMessage;
	Player* player = findPlayerOrNull(mUserID);

	if (player == nullptr)
	{
		moveMessage->Type = eMessageType::None;
		return;
	}
	int x = player->GetX();
	int y = player->GetY();
	int toX = x;
	int toY = y;

	if (GetAsyncKeyState(VK_LEFT) && 0 < x)
	{
		toX = x - 1;
	}
	else if (GetAsyncKeyState(VK_UP) && 0 < y)
	{
		toY = y - 1;
	}
	else if (GetAsyncKeyState(VK_RIGHT) && x < NUM_COLUMNS - 2)
	{
		toX = x + 1;
	}
	else if (GetAsyncKeyState(VK_DOWN) && y < NUM_ROWS - 1)
	{
		toY = y + 1;
	}
	else
	{
		moveMessage->Type = eMessageType::None;
		return;
	}
	player->Move(toX, toY);

	moveMessage->Type = eMessageType::MoveUser;
	moveMessage->UserID = player->GetID();
	moveMessage->X = toX;
	moveMessage->Y = toY;
}

void ApplicationManager::ProcessMessage(const Message& message)
{
	eMessageType type = static_cast<eMessageType>(message.Data[0]);
	int32_t userID = message.Data[1];
	int32_t x = message.Data[2];
	int32_t y = message.Data[3];

	switch (type)
	{
	case eMessageType::AssignID:
	{
		mUserID = userID;
		break;
	}
	case eMessageType::CreateUser:
	{
		mPlayerList.push_back(std::unique_ptr<Player>(new Player(x, y, userID)));
		break;
	}
	case eMessageType::DeleteUser:
	{
		auto iter = mPlayerList.begin();

		while (iter != mPlayerList.end())
		{
			if ((*iter)->GetID() == userID)
			{
				iter = mPlayerList.erase(iter);
				break;
			}
			else
			{
				++iter;
			}
		}
		break;
	}
	case eMessageType::MoveUser:
	{
		Player* player = findPlayerOrNull(userID);
		if (player != nullptr)
		{
			player->Move(x, y);
		}
		break;
	}
	case eMessageType::None:
	{
		break;
	}
	default:
		wprintf(L"Unhandled Message Type\n");
		break;
	}
}

const std::vector<std::unique_ptr<Player>>& ApplicationManager::GetPlayerList() const
{
	return mPlayerList;
}

Player* ApplicationManager::findPlayerOrNull(int32_t targetID) const
{
	for (auto& player : mPlayerList)
	{
		if (player->GetID() == targetID)
		{
			return player.get();
		}
	}
	return nullptr;
}

