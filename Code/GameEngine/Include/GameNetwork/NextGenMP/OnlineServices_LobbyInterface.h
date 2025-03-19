#pragma once

#include "NGMP_include.h"
#include "OnlineServices_RoomsInterface.h"

struct LobbyMember
{
	AsciiString m_strName = "NO_NAME";
	ENetworkConnectionState m_connectionState = ENetworkConnectionState::NOT_CONNECTED;
};

struct NGMP_LobbyInfo
{
	AsciiString strLobbyName;
	AsciiString strLobbyOwnerName;
	AsciiString strLobbyOwnerID;
	NGMP_ENATType NATType;
	AsciiString strMapName;
	int numMembers;
	int maxMembers;
};

class NGMP_OnlineServices_LobbyInterface
{
public:
	NGMP_OnlineServices_LobbyInterface();

	EOS_HLobbySearch m_SearchHandle = nullptr;
	std::function<void(std::vector<NGMP_LobbyInfo>)> m_PendingSearchLobbyCompleteCallback = nullptr;
	void SearchForLobbies(std::function<void()> onStartCallback, std::function<void(std::vector<NGMP_LobbyInfo>)> onCompleteCallback);

	// TODO_NGMP: We dont join right now (other than host)

	UnicodeString m_PendingCreation_LobbyName;
	UnicodeString m_PendingCreation_InitialMap;
	void CreateLobby(UnicodeString strLobbyName, UnicodeString strInitialMap, int initialMaxSize);

	UnicodeString GetCurrentLobbyDisplayName();

	void SendChatMessageToCurrentLobby(UnicodeString& strChatMsgUnicode);

	void InvokeCreateLobbyCallback(bool bSuccess)
	{
		for (auto cb : m_vecCreateLobby_PendingCallbacks)
		{
			// TODO_NGMP: Support failure
			cb(bSuccess);
		}
		m_vecCreateLobby_PendingCallbacks.clear();
	}

	// lobby roster
	std::function<void()> m_RosterNeedsRefreshCallback = nullptr;
	void RegisterForRosterNeedsRefreshCallback(std::function<void()> cb)
	{
		m_RosterNeedsRefreshCallback = cb;
	}

	void Tick()
	{
		if (m_pLobbyMesh != nullptr)
		{
			m_pLobbyMesh->Tick();
		}
	}

	LobbyMember* GetRoomMemberFromID(EOS_ProductUserId puid)
	{
		if (m_mapMembers.contains(puid))
		{
			return &m_mapMembers[puid];
		}

		return nullptr;
	}
	std::map<EOS_ProductUserId, LobbyMember>& GetMembersListForCurrentRoom()
	{
		NetworkLog("[NGMP] Refreshing network room roster");
		return m_mapMembers;
	}

	void RegisterForCreateLobbyCallback(std::function<void(bool)> callback)
	{
		m_vecCreateLobby_PendingCallbacks.push_back(callback);
	}

	void ApplyLocalUserPropertiesToCurrentNetworkRoom();

	bool IsHost();
	void SetCurrentLobbyID(const char* szLobbyID) { m_strCurrentLobbyID = std::string(szLobbyID); }

	void UpdateRoomDataCache();

	std::function<void(UnicodeString strMessage)> m_OnChatCallback = nullptr;
	void RegisterForChatCallback(std::function<void(UnicodeString strMessage)> cb)
	{
		m_OnChatCallback = cb;
	}

	void ResetCachedRoomData()
	{
		m_mapMembers.clear();

		if (m_RosterNeedsRefreshCallback != nullptr)
		{
			m_RosterNeedsRefreshCallback();
		}
	}

private:
	std::vector<std::function<void(bool)>> m_vecCreateLobby_PendingCallbacks = std::vector<std::function<void(bool)>>();

	std::string m_strCurrentLobbyID = "";

	// TODO_NGMP: cleanup
	NetworkMesh* m_pLobbyMesh = nullptr;

	std::map<EOS_ProductUserId, LobbyMember> m_mapMembers = std::map<EOS_ProductUserId, LobbyMember>();
};