#pragma once

#define INVALID_SIZE_MESSAGE "The size of the structure is invalid\n"
#define VALID_MESSAGE_SIZE (16)

enum class eMessageType : int32_t
{
	AssignID,
	CreateUser,
	DeleteUser,
	MoveUser,
	None
};

struct Message
{
	int32_t Data[4];
};

struct AssignIDMessage
{
	eMessageType Type;
	int32_t UserID;
	char Unused[8];
};

struct CreateUserMessage
{
	eMessageType Type;
	int32_t UserID;
	int32_t X;
	int32_t Y;
};

struct DeleteUserMessage
{
	eMessageType Type;
	int32_t UserID;
	char Unused[8];
};

struct MoveUserMessage
{
	eMessageType Type;
	int32_t UserID;
	int32_t X;
	int32_t Y;
};

void SetMessageData(Message* outMessage, eMessageType type, int32_t userID, int32_t x, int32_t y);

static_assert(sizeof(Message) == VALID_MESSAGE_SIZE, INVALID_SIZE_MESSAGE);
static_assert(sizeof(AssignIDMessage) == VALID_MESSAGE_SIZE, INVALID_SIZE_MESSAGE);
static_assert(sizeof(CreateUserMessage) == VALID_MESSAGE_SIZE, INVALID_SIZE_MESSAGE);
static_assert(sizeof(DeleteUserMessage) == VALID_MESSAGE_SIZE, INVALID_SIZE_MESSAGE);
static_assert(sizeof(MoveUserMessage) == VALID_MESSAGE_SIZE, INVALID_SIZE_MESSAGE);