#pragma once

#include "NGMP_include.h"
#include "OnlineServices_RoomsInterface.h"
#include "../GameInfo.h"

class LobbyMember : public NetworkMemberBase
{

};

struct NGMP_LobbyInfo
{
	AsciiString strLobbyName;
	AsciiString strLobbyOwnerName;
	AsciiString strLobbyOwnerID;
	NGMP_ENATType NATType;
	AsciiString strMapDisplayName;
	AsciiString strMapPath;
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

	NextGenTransport* m_transport = nullptr;
	void InitGameTransport()
	{
		if (m_transport != nullptr)
		{
			delete m_transport;
			m_transport = nullptr;
		}
		m_transport = new NextGenTransport;
		m_transport->reset();
		m_transport->init(0, 0); // we dont care about ip/port anymore
	}

	NextGenTransport* GetGameTransport()
	{
		return m_transport;
	}

	// TODO_NGMP: We dont join right now (other than host)

	UnicodeString m_PendingCreation_LobbyName;
	UnicodeString m_PendingCreation_InitialMapDisplayName;
	AsciiString m_PendingCreation_InitialMapPath;
	void CreateLobby(UnicodeString strLobbyName, UnicodeString strInitialMapName, AsciiString strInitialMapPath, int initialMaxSize);

	void OnJoinedOrCreatedLobby();

	UnicodeString GetCurrentLobbyDisplayName();
	UnicodeString GetCurrentLobbyMapDisplayName();
	AsciiString GetCurrentLobbyMapPath();

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

	NGMPGame* GetCurrentGame()
	{
		return m_pGameInst;
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

	LobbyMember* GetRoomMemberFromIndex(int index)
	{
		// TODO_NGMP: Optimize data structure
		if (index < m_mapMembers.size())
		{
			int i = 0;
			for (auto kvPair : m_mapMembers)
			{
				if (i == index)
				{
					return &kvPair.second;
				}

				++i;
			}
		}

		return nullptr;
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

	void SendToMesh(NetworkPacket& packet)
	{
		std::vector<EOS_ProductUserId> vecUsers;
		for (auto kvPair : m_mapMembers)
		{
			vecUsers.push_back(kvPair.first);
		}

		if (m_pLobbyMesh != nullptr)
		{
			m_pLobbyMesh->SendToMesh(packet, vecUsers);
		}
	}

private:
	std::vector<std::function<void(bool)>> m_vecCreateLobby_PendingCallbacks = std::vector<std::function<void(bool)>>();

	std::string m_strCurrentLobbyID = "";

	// TODO_NGMP: cleanup
	NetworkMesh* m_pLobbyMesh = nullptr;

	NGMPGame* m_pGameInst = nullptr;

	std::map<EOS_ProductUserId, LobbyMember> m_mapMembers = std::map<EOS_ProductUserId, LobbyMember>();
};