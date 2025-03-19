#pragma once
#pragma once

enum ENetworkConnectionState
{
	NOT_CONNECTED,
	CONNECTED_DIRECT,
	CONNECTED_RELAYED
};

enum ENetworkMeshType : uint8_t
{
	NETWORK_ROOM = 0,
	GAME_LOBBY = 1
};