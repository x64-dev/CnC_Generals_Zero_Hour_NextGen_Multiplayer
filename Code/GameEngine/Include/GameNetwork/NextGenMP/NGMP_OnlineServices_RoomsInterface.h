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

class NGMP_OnlineServices_NetworkRoomMesh
{
public:
	NGMP_OnlineServices_NetworkRoomMesh()
	{

	}

	~NGMP_OnlineServices_NetworkRoomMesh()
	{

	}

	void SendHelloMsg(EOS_ProductUserId targetUser);
	void SendHelloAckMsg(EOS_ProductUserId targetUser);
	void SendToMesh(NetworkPacket& packet, std::vector<EOS_ProductUserId> vecTargetUsers);
	void ConnectToMesh(const char* szRoomID);

	void Tick();

private:
	EOS_P2P_SocketId m_SockID;

	// TODO_NGMP: Everywhere we use notifications, we should check if after creating it it is invalid, if so, error
};

class NGMP_OnlineServices_RoomsInterface
{
public:
	NGMP_OnlineServices_RoomsInterface();

	void UpdateRoomDataCache();

	EOS_HLobbySearch m_SearchHandle = nullptr;
	
	std::function<void()> m_PendingRoomJoinCompleteCallback = nullptr;
	void JoinRoom(int roomIndex, std::function<void()> onStartCallback, std::function<void()> onCompleteCallback);

	std::function<void(UnicodeString strMessage)> m_OnChatCallback = nullptr;
	void RegisterForChatCallback(std::function<void(UnicodeString strMessage)> cb)
	{
		m_OnChatCallback = cb;
	}

	std::function<void()> m_RosterNeedsRefreshCallback = nullptr;
	void RegisterForRosterNeedsRefreshCallback(std::function<void()> cb)
	{
		m_RosterNeedsRefreshCallback = cb;
	}

	

	void CreateRoom(int roomIndex);

	std::map<EOS_ProductUserId, NetworkRoomMember>& GetMembersListForCurrentRoom();

	EOS_HLobbyDetails m_currentRoomDetailsHandle = nullptr;

	// Chat
	void SendChatMessageToCurrentRoom(UnicodeString& strChatMsg);

	void ResetCachedRoomData()
	{
		m_mapMembers.clear();
	
		if (m_RosterNeedsRefreshCallback != nullptr)
		{
			m_RosterNeedsRefreshCallback();
		}
	}

	void Tick()
	{
		m_netRoomMesh.Tick();
	}

private:
	void ApplyLocalUserPropertiesToCurrentNetworkRoom();

private:
	int m_CurrentRoomID = -1;
	NGMP_OnlineServices_NetworkRoomMesh m_netRoomMesh;

	std::map<EOS_ProductUserId, NetworkRoomMember> m_mapMembers = std::map<EOS_ProductUserId, NetworkRoomMember>();
};