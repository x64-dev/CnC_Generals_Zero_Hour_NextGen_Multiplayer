#pragma once

#include "NGMP_include.h"

struct NGMP_LobbyInfo
{
	AsciiString strLobbyName;
	AsciiString strLobbyOwner;
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
	void CreateLobby(UnicodeString strLobbyName);

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

private:
	std::vector<std::function<void(bool)>> m_vecCreateLobby_PendingCallbacks = std::vector<std::function<void(bool)>>();
};