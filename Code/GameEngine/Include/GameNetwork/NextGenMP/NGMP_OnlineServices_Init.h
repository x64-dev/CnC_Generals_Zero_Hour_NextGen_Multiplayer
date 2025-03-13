#pragma once
#include <windows.h>
#include <processenv.h>
#include <string.h>
#include <eos_common.h>


#include <eos_p2p_types.h>
#include <eos_auth_types.h>
#include <eos_connect_types.h>
#include <eos_auth.h>
#include <eos_connect.h>
#include <eos_sdk.h>
#include <eos_p2p.h>
#include <eos_types.h>
#include <eos_sessions.h>
#include <eos_lobby.h>
#include <eos_lobby_types.h>
#include <eos_metrics.h>
#include <eos_userinfo.h>
#include <eos_sanctions.h>
#include <eos_playerdatastorage.h>
#include <eos_playerdatastorage_types.h>
#include <eos_achievements.h>
#include <eos_logging.h>
#include <eos_titlestorage.h>
#include <eos_titlestorage_types.h>
#include <eos_friends.h>
#include <eos_friends_types.h>
#include <eos_stats.h>
#include <eos_stats_types.h>
#include <eos_reports.h>
#include <Windows/eos_windows.h>
#include <eos_integratedplatform_types.h>


#include <steam_api.h>

#include <minwindef.h>
#include <steam_api_common.h>
#include <vector>
#include <functional>
#include <string>
#include "Common/UnicodeString.h"

#pragma comment(lib, "EOSSDK-Win64-Shipping.lib")
#pragma comment(lib, "steam_api64.lib")
#pragma comment(lib, "sdkencryptedappticket64.lib")

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
		// TODO_NGMP: Correct values
		EOS_HLobby lobbyHandle = EOS_Platform_GetLobbyInterface(m_EOSPlatformHandle);

		EOS_Lobby_CreateLobbyOptions* createLobbyOpts = new EOS_Lobby_CreateLobbyOptions();
		createLobbyOpts->ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
		createLobbyOpts->LocalUserId = m_EOSUserID;
		createLobbyOpts->MaxLobbyMembers = 4;
		createLobbyOpts->PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
		createLobbyOpts->bPresenceEnabled = false;
		createLobbyOpts->BucketId = "TODO_NGMP";
		createLobbyOpts->bDisableHostMigration = true; // Generals doesnt support host migration during lobby... maybe we should fix that
		createLobbyOpts->bEnableRTCRoom = false;
		createLobbyOpts->LocalRTCOptions = nullptr;
		createLobbyOpts->bEnableJoinById = true;
		createLobbyOpts->bRejoinAfterKickRequiresInvite = false;
		createLobbyOpts->AllowedPlatformIds = nullptr;
		createLobbyOpts->AllowedPlatformIdsCount = 0;
		createLobbyOpts->bCrossplayOptOut = false;
		createLobbyOpts->RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_ManualJoin;
		
		EOS_Lobby_CreateLobby(lobbyHandle, createLobbyOpts, nullptr, [](const EOS_Lobby_CreateLobbyCallbackInfo* Data)
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					OutputDebugString("Lobby created!\n");
				}
				else
				{
					OutputDebugString("Failed to create lobby!\n");
				}

				// TODO_NGMP: Impl
				NGMP_OnlineServicesManager::GetInstance()->InvokeCreateLobbyCallback(Data->ResultCode == EOS_EResult::EOS_Success);
			});
	}

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
	EOS_ProductUserId m_EOSUserID = nullptr;

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