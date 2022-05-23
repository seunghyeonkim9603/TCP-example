#pragma once

class BoundingRect final
{
public:
    BoundingRect(const Point topLeft, const int width, const int height);
    ~BoundingRect() = default;
    BoundingRect(const BoundingRect& other) = default;
    BoundingRect& operator=(const BoundingRect& other) = default;

    int GetWidth() const;
    int GetHeight() const;
    Point GetTopLeft() const;
    Point GetBottomRight() const;

    bool Contains(const Point& point) const;
    bool Contains(const BoundingRect& rect) const;

private:
    Point mTopLeft;
    Point mBottomRight;
};