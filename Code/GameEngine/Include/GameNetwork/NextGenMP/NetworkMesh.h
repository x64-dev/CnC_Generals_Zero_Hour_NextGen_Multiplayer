#pragma once

#include "NGMP_include.h"

class NetworkMesh
{
public:
	NetworkMesh(ENetworkMeshType meshType)
	{
		m_meshType = meshType;
	}

	~NetworkMesh()
	{

	}

	void SendHelloMsg(EOS_ProductUserId targetUser);
	void SendHelloAckMsg(EOS_ProductUserId targetUser);
	void SendToMesh(NetworkPacket& packet, std::vector<EOS_ProductUserId> vecTargetUsers);
	void ConnectToMesh(const char* szRoomID);

	void Tick();

private:
	EOS_P2P_SocketId m_SockID;

	ENetworkMeshType m_meshType;

	// TODO_NGMP: Everywhere we use notifications, we should check if after creating it it is invalid, if so, error
};