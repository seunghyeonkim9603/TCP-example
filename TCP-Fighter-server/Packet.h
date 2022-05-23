#pragma once

#include "PacketDefine.h"

#pragma pack(push, 1)

struct PacketHeader
{

	__int8 Code;
	__int8 Size;
	__int8 Type;

	void Init(__int8 size, ePacketType type)
	{
		Code = (__int8)PACKET_CODE;
		Size = size;
		Type = (__int8)type;
	}
};
static_assert(sizeof(PacketHeader) == PACKET_HEADER_SIZE, "");

struct SCCreateMyCharacterPacket : PacketHeader
{

	__int32 ID;
	__int8 Direction;
	__int16 X;
	__int16 Y;
	__int8 HP;

	void SetData(__int32 Id, __int8 direction, __int16 x, __int16 y, __int8 hp)
	{
		Init(10, ePacketType::SCCreateMyCharacter);
		ID = Id;
		Direction = direction;
		X = x;
		Y = y;
		HP = hp;
	}
};
static_assert(sizeof(SCCreateMyCharacterPacket) == 13, "");


struct SCCreateOtherCharacterPacket : PacketHeader
{

	__int32 ID;
	__int8 Direction;
	__int16 X;
	__int16 Y;
	__int8 HP;

	void SetData(__int32 Id, __int8 direction, __int16 x, __int16 y, __int8 hp)
	{
		Init(10, ePacketType::SCCreateOtherCharacter);
		ID = Id;
		Direction = direction;
		X = x;
		Y = y;
		HP = hp;
	}
};
static_assert(sizeof(SCCreateOtherCharacterPacket) == 13, "");


struct SCDeleteCharacterPacket : PacketHeader
{
	__int32 ID;

	void SetData(__int32 Id)
	{
		Init(4, ePacketType::SCDeleteCharacter);
		ID = Id;
	}
};
static_assert(sizeof(SCDeleteCharacterPacket) == 7, "");


struct CSMoveStartPacket
{
	__int8 Direction;
	__int16 X;
	__int16 Y;
};
static_assert(sizeof(CSMoveStartPacket) == 5, "");


struct SCMoveStartPacket : PacketHeader
{

	__int32 ID;
	__int8 Direction;
	__int16 X;
	__int16 Y;

	void SetData(__int32 Id, __int8 direction, __int16 x, __int16 y)
	{
		Init(9, ePacketType::SCMoveStart);
		ID = Id;
		Direction = direction;
		X = x;
		Y = y;
	}
};
static_assert(sizeof(SCMoveStartPacket) == 12, "");


struct CSMoveStopPacket
{
	__int8 Direction;
	__int16 X;
	__int16 Y;
};
static_assert(sizeof(CSMoveStopPacket) == 5, "");


struct SCMoveStopPacket : PacketHeader
{

	__int32 ID;
	__int8 Direction;
	__int16 X;
	__int16 Y;

	void SetData(__int32 Id, __int8 direction, __int16 x, __int16 y)
	{
		Init(9, ePacketType::SCMoveStop);
		ID = Id;
		Direction = direction;
		X = x;
		Y = y;
	}
};
static_assert(sizeof(SCMoveStopPacket) == 12, "");


struct CSAttack1Packet
{
	__int8 Direction;
	__int16 X;
	__int16 Y;
};
static_assert(sizeof(CSAttack1Packet) == 5, "");


struct SCAttack1Packet : PacketHeader
{

	__int32 ID;
	__int8 Direction;
	__int16 X;
	__int16 Y;

	void SetData(__int32 Id, __int8 direction, __int16 x, __int16 y)
	{
		Init(9, ePacketType::SCAttack1);
		ID = Id;
		Direction = direction;
		X = x;
		Y = y;
	}
};
static_assert(sizeof(SCAttack1Packet) == 12, "");


struct CSAttack2Packet
{
	__int8 Direction;
	__int16 X;
	__int16 Y;
};
static_assert(sizeof(CSAttack2Packet) == 5, "");


struct SCAttack2Packet : PacketHeader
{
	__int32 ID;
	__int8 Direction;
	__int16 X;
	__int16 Y;

	void SetData(__int32 Id, __int8 direction, __int16 x, __int16 y)
	{
		Init(9, ePacketType::SCAttack2);
		ID = Id;
		Direction = direction;
		X = x;
		Y = y;
	}
};
static_assert(sizeof(SCAttack2Packet) == 12, "");


struct CSAttack3Packet
{
	__int8 Direction;
	__int16 X;
	__int16 Y;
};
static_assert(sizeof(CSAttack3Packet) == 5, "");


struct SCAttack3Packet : PacketHeader
{
	__int32 ID;
	__int8 Direction;
	__int16 X;
	__int16 Y;

	void SetData(__int32 Id, __int8 direction, __int16 x, __int16 y)
	{
		Init(9, ePacketType::SCAttack3);
		ID = Id;
		Direction = direction;
		X = x;
		Y = y;
	}
};
static_assert(sizeof(SCAttack3Packet) == 12, "");


struct SCDamagePacket : PacketHeader
{
	__int32 AttackerID;
	__int32 TargetID;
	__int8 TargetHP;

	void SetData(__int32 attackerID, __int32 targetID, __int8 targetHP)
	{
		Init(9, ePacketType::SCDamage);
		AttackerID = attackerID;
		TargetID = targetID;
		TargetHP = targetHP;
	}
};
static_assert(sizeof(SCDamagePacket) == 12, "");

#pragma pack(pop)
