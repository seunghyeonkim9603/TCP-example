#include <cstdint>

#include "Message.h"

void SetMessageData(Message* outMessage, eMessageType type, int32_t userID, int32_t x, int32_t y)
{
	outMessage->Data[0] = (int32_t)type;
	outMessage->Data[1] = userID;
	outMessage->Data[2] = x;
	outMessage->Data[3] = y;
}