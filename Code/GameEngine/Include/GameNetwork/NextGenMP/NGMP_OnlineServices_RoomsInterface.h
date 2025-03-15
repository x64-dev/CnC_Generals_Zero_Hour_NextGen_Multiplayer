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
	NGMP_OnlineServices_RoomsInterface();

	void UpdateRoomDataCache();

	EOS_HLobbySearch m_SearchHandle = nullptr;
	
	std::function<void()> m_PendingRoomJoinCompleteCallback = nullptr;
	void JoinRoom(int roomIndex, std::function<void()> onStartCallback, std::function<void()> onCompleteCallback);

	void CreateRoom(int roomIndex);

	std::map<EOS_ProductUserId, NetworkRoomMember> GetMembersListForCurrentRoom();

	EOS_HLobbyDetails m_currentRoomDetailsHandle = nullptr;

	// Chat
	void SendChatMessageToCurrentRoom(UnicodeString& strChatMsg);

	void ResetCachedRoomData() { m_mapMembers.clear(); }

private:
	void ApplyLocalUserPropertiesToCurrentNetworkRoom();

private:
	int m_CurrentRoomID = -1;

	std::map<EOS_ProductUserId, NetworkRoomMember> m_mapMembers = std::map<EOS_ProductUserId, NetworkRoomMember>();
};