#pragma once
#include <windows.h>
#include <processenv.h>
#include <string.h>
#include "Vendor/EpicOnlineServices/Include/eos_common.h"


#include "Vendor/EpicOnlineServices/Include/eos_p2p_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_auth_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_connect_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_auth.h"
#include "Vendor/EpicOnlineServices/Include/eos_connect.h"
#include "Vendor/EpicOnlineServices/Include/eos_sdk.h"
#include "Vendor/EpicOnlineServices/Include/eos_p2p.h"
#include "Vendor/EpicOnlineServices/Include/eos_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_sessions.h"
#include "Vendor/EpicOnlineServices/Include/eos_lobby.h"
#include "Vendor/EpicOnlineServices/Include/eos_lobby_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_metrics.h"
#include "Vendor/EpicOnlineServices/Include/eos_userinfo.h"
#include "Vendor/EpicOnlineServices/Include/eos_sanctions.h"
#include "Vendor/EpicOnlineServices/Include/eos_playerdatastorage.h"
#include "Vendor/EpicOnlineServices/Include/eos_playerdatastorage_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_achievements.h"
#include "Vendor/EpicOnlineServices/Include/eos_logging.h"
#include "Vendor/EpicOnlineServices/Include/eos_titlestorage.h"
#include "Vendor/EpicOnlineServices/Include/eos_titlestorage_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_friends.h"
#include "Vendor/EpicOnlineServices/Include/eos_friends_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_stats.h"
#include "Vendor/EpicOnlineServices/Include/eos_stats_types.h"
#include "Vendor/EpicOnlineServices/Include/eos_reports.h"
#include "Vendor/EpicOnlineServices/Include/Windows/eos_windows.h"
#include "Vendor/EpicOnlineServices/Include/eos_integratedplatform_types.h"


#include "Vendor/Steamworks/include/steam_api.h"

#include <minwindef.h>
#include "Vendor/Steamworks/include/steam_api_common.h"
#include <vector>
#include <functional>
#include <string>
#include "Common/UnicodeString.h"

#pragma comment(lib, "NextGenMP//Vendor//EpicOnlineServices//Lib//EOSSDK-Win64-Shipping.lib")
#pragma comment(lib, "NextGenMP//Vendor//Steamworks//Lib//steam_api64.lib")
#pragma comment(lib, "NextGenMP//Vendor//Steamworks//Lib//sdkencryptedappticket64.lib")

class NetworkRoom
{
public:
	NetworkRoom(int roomID, wchar_t* strRoomName)
	{
		m_RoomID = roomID;
		m_strRoomName = UnicodeString(strRoomName);
	}

	~NetworkRoom()
	{

	}

	int GetRoomID() const { return m_RoomID; }
	UnicodeString GetRoomName() const { return m_strRoomName; }

private:
	int m_RoomID;
	UnicodeString m_strRoomName;
};

class NGMP_OnlineServicesManager
{
private:
	static NGMP_OnlineServicesManager* m_pOnlineServicesManager;

public:

	NGMP_OnlineServicesManager();
	
	static NGMP_OnlineServicesManager* GetInstance()
	{
		return m_pOnlineServicesManager;
	}
	
	void SendNetworkRoomChat(UnicodeString msg);

	void Init();

	void Auth();

	void Tick();

	void LoginToEpic();

	void OnEpicLoginComplete(EOS_ProductUserId userID);

	void RegisterForLoginCallback(std::function<void(bool)> callback)
	{
		m_vecLogin_PendingCallbacks.push_back(callback);
	}

	void CreateLobby()
	{
		// TODO_NGMP: Impl
		for (auto cb : m_vecCreateLobby_PendingCallbacks)
		{
			// TODO_NGMP: Support failure
			cb(true);
		}
		m_vecCreateLobby_PendingCallbacks.clear();
	}

	void RegisterForCreateLobbyCallback(std::function<void(bool)> callback)
	{
		m_vecCreateLobby_PendingCallbacks.push_back(callback);
	}

	STEAM_CALLBACK(NGMP_OnlineServicesManager, OnAuthSessionTicketResponse, GetTicketForWebApiResponse_t);

	EOS_HPlatform GetEOSPlatformHandle() {
		return m_EOSPlatformHandle;
	}

	std::string GetDisplayName()
	{
		return m_strDisplayName;
	}


	//bool m_bPendingLoginFlag = false;

	std::vector<NetworkRoom> GetGroupRooms()
	{
		return m_vecRooms;
	}

private:
	EOS_HPlatform m_EOSPlatformHandle = nullptr;

	std::vector<uint8> m_vecSteamAuthSessionTicket;

	std::vector<std::function<void(bool)>> m_vecLogin_PendingCallbacks = std::vector<std::function<void(bool)>>();
	std::vector<std::function<void(bool)>> m_vecCreateLobby_PendingCallbacks = std::vector<std::function<void(bool)>>();

	std::string m_strDisplayName = "NO_USER";

	// TODO_NGMP: Get this from title storage or session query instead
	std::vector<NetworkRoom> m_vecRooms =
	{
		NetworkRoom(0, L"First Ever Room!"),
		NetworkRoom(1, L"Another room!")
	};
};