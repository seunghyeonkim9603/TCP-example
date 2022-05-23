#include <memory>
#include <unordered_map>
#include <WinSock2.h>

#include "Action.h"
#include "Direction.h"
#include "Point.h"
#include "BoundingRect.h"
#include "RingBuffer.h"
#include "User.h"
#include "Quadrant.h"

Quadrant::Quadrant(const BoundingRect& rect)
	:mBoundingRect(rect)
{
	createChildren();
}

Quadrant::~Quadrant()
{
	delete mTopLeft;
	delete mTopRight;
	delete mBottomLeft;
	delete mBottomRight;
}

bool Quadrant::TryInsert(User* user)
{
    if (!mBoundingRect.Contains(user->Position))
    {
        return false;
    }
    mUsers.insert({ user->ID, user });
    
    if (mTopLeft != nullptr)
    {
        mTopLeft->TryInsert(user);
        mTopRight->TryInsert(user);
        mBottomLeft->TryInsert(user);
        mBottomRight->TryInsert(user);
    }
	return true;
}

bool Quadrant::TryDelete(const int userID)
{
    if (mUsers.find(userID) == mUsers.end())
    {
        return false;
    }
    mUsers.erase(userID);

    if (mTopLeft != nullptr)
    {
        mTopLeft->TryDelete(userID);
        mTopRight->TryDelete(userID);
        mBottomLeft->TryDelete(userID);
        mBottomRight->TryDelete(userID);
    }
	return true;
}

bool Quadrant::TryRelocate(const int userID)
{
    auto userIter = mUsers.find(userID);
    if (userIter == mUsers.end())
    {
        return false;
    }
    User* user = userIter->second;

    TryDelete(userID);
    TryInsert(user);

    return true;
}

std::unordered_map<int, User*>& Quadrant::GetUsers(const BoundingRect& rect)
{
    if (mTopLeft == nullptr)
    {
        return mUsers;
    }

    if (mTopLeft->mBoundingRect.Contains(rect))
    {
        return mTopLeft->GetUsers(rect);
    }
    if (mTopRight->mBoundingRect.Contains(rect))
    {
        return mTopRight->GetUsers(rect);
    }
    if (mBottomLeft->mBoundingRect.Contains(rect))
    {
        return mBottomLeft->GetUsers(rect);
    }
    if (mBottomRight->mBoundingRect.Contains(rect))
    {
        return mBottomRight->GetUsers(rect);
    }
	return mUsers;
}

void Quadrant::createChildren()
{
    const int width = mBoundingRect.GetWidth();
    const int height = mBoundingRect.GetHeight();

    if (width < 2 * MIN_QUAD_DIMENSION || height < 2 * MIN_QUAD_DIMENSION)
    {
        return;
    }

    const int x1 = mBoundingRect.GetTopLeft().X;
    const int y1 = mBoundingRect.GetTopLeft().Y;
    const int x2 = mBoundingRect.GetBottomRight().X;
    const int y2 = mBoundingRect.GetBottomRight().Y;

    int midX = (x1 + x2) / 2;
    int midY = (y1 + y2) / 2;

    Point p1 = { x1, y1 };
    Point p2 = { midX, midY };

    BoundingRect topLeftRect(p1, p2.X - p1.X, p2.Y - p1.Y);

    mTopLeft = new Quadrant(topLeftRect);

    p1 = { midX, y1 };
    p2 = { x2, midY };
    BoundingRect topRightRect(p1, p2.X - p1.X, p2.Y - p1.Y);

    mTopRight = new Quadrant(topRightRect);

    p1 = { x1, midY };
    p2 = { midX, y2 };
    BoundingRect bottomLeftRect(p1, p2.X - p1.X, p2.Y - p1.Y);

    mBottomLeft = new Quadrant(bottomLeftRect);

    p1 = { midX, midY };
    p2 = { x2, y2 };
    BoundingRect bottomRightRect(p1, p2.X - p1.X, p2.Y - p1.Y);

    mBottomRight = new Quadrant(bottomRightRect);
}
