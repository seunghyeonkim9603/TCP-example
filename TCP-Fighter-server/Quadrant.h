#pragma once

class Quadrant final
{
public:
    Quadrant() = delete;
    ~Quadrant();
    Quadrant(const BoundingRect& rect);
    Quadrant(const Quadrant& other) = delete;

    bool TryInsert(User* user);
    bool TryDelete(const int userID);
    bool TryRelocate(const int userID);

    std::unordered_map<int, User*>& GetUsers(const BoundingRect& rect);

private:
    void createChildren();

private:
    enum { MIN_QUAD_DIMENSION = 10};

    BoundingRect mBoundingRect;

    Quadrant* mTopLeft = nullptr;
    Quadrant* mTopRight = nullptr;
    Quadrant* mBottomLeft = nullptr;
    Quadrant* mBottomRight = nullptr;

    std::unordered_map<int, User*> mUsers;
};