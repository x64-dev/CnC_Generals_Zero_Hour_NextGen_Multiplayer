#include "GameNetwork/NextGenMP/Packets/NetworkPacket_NetRoom_Hello.h"
#include "GameNetwork/NextGenMP/NetworkBitstream.h"

NetRoom_HelloPacket::NetRoom_HelloPacket() : NetworkPacket(EPacketReliability::PACKET_RELIABILITY_RELIABLE_ORDERED)
{

}

NetRoom_HelloPacket::NetRoom_HelloPacket(CBitStream& bitstream) : NetworkPacket(bitstream)
{

}

CBitStream* NetRoom_HelloPacket::Serialize()
{
	CBitStream* pBitstream = new CBitStream(EPacketID::PACKET_ID_NET_ROOM_HELLO);
	return pBitstream;
}