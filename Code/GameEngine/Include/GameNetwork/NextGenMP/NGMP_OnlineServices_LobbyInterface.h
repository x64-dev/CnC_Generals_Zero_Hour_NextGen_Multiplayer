#pragma once

#include "NGMP_include.h"

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
	EOS_HLobbySearch m_SearchHandle = nullptr;
	std::function<void(std::vector<NGMP_LobbyInfo>)> m_PendingSearchLobbyCompleteCallback = nullptr;
	void SearchForLobbies(std::function<void()> onStartCallback, std::function<void(std::vector<NGMP_LobbyInfo>)> onCompleteCallback);

	UnicodeString m_PendingCreation_LobbyName;
	UnicodeString m_PendingCreation_InitialMap;
	void CreateLobby(UnicodeString strLobbyName, UnicodeString strInitialMap, int initialMaxSize);

	void InvokeCreateLobbyCallback(bool bSuccess)
	{
		for (auto cb : m_vecCreateLobby_PendingCallbacks)
		{
			// TODO_NGMP: Support failure
			cb(bSuccess);
		}
		m_vecCreateLobby_PendingCallbacks.clear();
	}

	void RegisterForCreateLobbyCallback(std::function<void(bool)> callback)
	{
		m_vecCreateLobby_PendingCallbacks.push_back(callback);
	}

	bool IsHost();
	void SetCurrentLobbyID(const char* szLobbyID) { m_strCurrentLobbyID = std::string(szLobbyID); }

private:
	std::vector<std::function<void(bool)>> m_vecCreateLobby_PendingCallbacks = std::vector<std::function<void(bool)>>();

	std::string m_strCurrentLobbyID = "";
};