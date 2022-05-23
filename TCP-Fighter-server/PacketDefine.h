#pragma once

#define PACKET_HEADER_SIZE (3)
#define PACKET_CODE (0x89)

enum class ePacketType : unsigned char
{
	SCCreateMyCharacter = 0,
	SCCreateOtherCharacter = 1,
	SCDeleteCharacter = 2,
	CSMoveStart = 10,
	SCMoveStart = 11,
	CSMoveStop = 12,
	SCMoveStop = 13,
	CSAttack1 = 20,
	SCAttack1 = 21,
	CSAttack2 = 22,
	SCAttack2 = 23,
	CSAttack3  = 24,
	SCAttack3 = 25,
	SCDamage = 30
};
