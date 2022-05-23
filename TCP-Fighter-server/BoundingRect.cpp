#include <assert.h>
#include <cmath>

#include "Point.h"
#include "BoundingRect.h"

BoundingRect::BoundingRect(const Point topLeft, const int width, const int height)
{
    assert(width >= 0);
    assert(height >= 0);

    mTopLeft = topLeft;
    mBottomRight.X = topLeft.X + width;
    mBottomRight.Y = topLeft.Y + height;
}

int BoundingRect::GetWidth() const
{
    const int y1 = mTopLeft.Y;
    const int y2 = mBottomRight.Y;

    return abs(y1 - y2);
}

int BoundingRect::GetHeight() const
{
    const int x1 = mTopLeft.X;
    const int x2 = mBottomRight.X;

    return abs(x1 - x2);
}

Point BoundingRect::GetTopLeft() const
{
    return mTopLeft;
}

Point BoundingRect::GetBottomRight() const
{
    return mBottomRight;
}

bool BoundingRect::Contains(const BoundingRect& rect) const
{
    const int x1 = mTopLeft.X;
    const int x2 = mBottomRight.X;
    const int y1 = mTopLeft.Y;
    const int y2 = mBottomRight.Y;

    const int otherX1 = rect.mTopLeft.X;
    const int otherX2 = rect.mBottomRight.X;
    const int otherY1 = rect.mTopLeft.Y;
    const int otherY2 = rect.mBottomRight.Y;

    return x1 <= otherX1
        && x2 >= otherX2
        && y1 <= otherY1
        && y2 >= otherY2;
}

bool BoundingRect::Contains(const Point& point) const
{
    const int pX = point.X;
    const int pY = point.Y;

    return pX >= mTopLeft.X
        && pX <= mBottomRight.X
        && pY >= mTopLeft.Y
        && pY <= mBottomRight.Y;
}
