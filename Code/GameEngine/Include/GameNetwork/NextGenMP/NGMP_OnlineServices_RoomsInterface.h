#pragma once

#include "NGMP_include.h"

struct NGMP_RoomInfo
{
	int numMembers;
	int maxMembers;
};

struct NetworkRoomMember
{
	AsciiString m_strName;
};

class NGMP_OnlineServices_RoomsInterface
{
public:
	EOS_HLobbySearch m_SearchHandle = nullptr;
	
	std::function<void()> m_PendingRoomJoinCompleteCallback = nullptr;
	void JoinRoom(int roomIndex, std::function<void()> onStartCallback, std::function<void()> onCompleteCallback);

	void CreateRoom(int roomIndex);

	std::vector<NetworkRoomMember> GetMembersListForCurrentRoom();

	EOS_HLobbyDetails m_currentRoomDetailsHandle = nullptr;

private:
	int m_CurrentRoomID = -1;
};