#pragma once

#include "GameNetwork/NextGenMP/NetworkPacket.h"

class NetRoom_HelloAckPacket : public NetworkPacket
{
public:
	NetRoom_HelloAckPacket();

	NetRoom_HelloAckPacket(CBitStream& bitstream);

	virtual CBitStream* Serialize() override;
};