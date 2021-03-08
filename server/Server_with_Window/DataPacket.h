#pragma once
#include "common.h"

#define PACKET_REQ_CMD 1 //send command
#define PACKET_RLY_CMD 2 //receive result
#define PACKET_RLY_KEYBOARD 3 //keyboard records

typedef struct DataPacket
{
	BYTE type;
	WORD len;
	char data[0];
}DataPacket;