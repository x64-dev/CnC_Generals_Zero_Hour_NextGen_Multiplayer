#pragma once

#include "GameNetwork/NextGenMP/NetworkPacket.h"

class NetRoom_HelloPacket : public NetworkPacket
{
public:
	NetRoom_HelloPacket();

	NetRoom_HelloPacket(CBitStream& bitstream);

	virtual CBitStream* Serialize() override;
};