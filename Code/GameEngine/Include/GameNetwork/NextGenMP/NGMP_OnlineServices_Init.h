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
#include "Common/AsciiString.h"

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

struct NGMP_LobbyInfo
{
	AsciiString strLobbyName;
	AsciiString strLobbyOwner;
	int numMembers;
	int maxMembers;
};

enum NGMP_ENATType : uint8_t
{
	NAT_TYPE_UNDETERMINED,
	NAT_TYPE_OPEN,
	NAT_TYPE_MODERATE,
	NAT_TYPE_STRICT
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

	EOS_HLobbySearch m_SearchHandle = nullptr;
	std::function<void(std::vector<NGMP_LobbyInfo>)> m_PendingSearchLobbyCompleteCallback = nullptr;
	void SearchForLobbies(std::function<void()> onStartCallback, std::function<void(std::vector<NGMP_LobbyInfo>)> onCompleteCallback)
	{
		m_PendingSearchLobbyCompleteCallback = onCompleteCallback;

		EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(m_EOSPlatformHandle);

		EOS_Lobby_CreateLobbySearchOptions CreateSearchOptions = {};
		CreateSearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
		CreateSearchOptions.MaxResults = 200;

		
		EOS_EResult Result = EOS_Lobby_CreateLobbySearch(LobbyHandle, &CreateSearchOptions, &m_SearchHandle);

		if (Result == EOS_EResult::EOS_Success)
		{
			EOS_LobbySearch_FindOptions FindOptions = {};
			FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
			FindOptions.LocalUserId = m_EOSUserID;

			// bucket id param
			{
				/*
				EOS_LobbySearch_SetParameterOptions ParamOptions = {};
				EOS_Lobby_AttributeData AttributeData;
				AttributeData.Key = "BUCKET";
				AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
				AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;
				AttributeData.Value.AsUtf8 = "TODO_NGMP"; // TODO_NGMP: Proper value
				ParamOptions.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;
				ParamOptions.ComparisonOp = EOS_EComparisonOp::EOS_CO_EQUAL;
				ParamOptions.Parameter = &AttributeData;
				EOS_EResult result = EOS_LobbySearch_SetParameter(m_SearchHandle, &ParamOptions);
				if (result != EOS_EResult::EOS_Success)
				{
					OutputDebugString("Failed to set search param");
				}
				*/
			}

			// version param
			{
				EOS_LobbySearch_SetParameterOptions ParamOptions = {};
				EOS_Lobby_AttributeData AttributeData;
				AttributeData.Key = "VERSION";
				AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
				AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_INT64;
				AttributeData.Value.AsInt64 = 1337; // TODO_NGMP: Proper value

				ParamOptions.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;
				ParamOptions.ComparisonOp = EOS_EComparisonOp::EOS_CO_EQUAL;
				ParamOptions.Parameter = &AttributeData;

				EOS_EResult result = EOS_LobbySearch_SetParameter(m_SearchHandle, &ParamOptions);
				if (result != EOS_EResult::EOS_Success)
				{
					OutputDebugString("Failed to set search param");
				}
			}

			if (onStartCallback != nullptr)
			{
				onStartCallback();
			}

			// TODO_NGMP: Make sure 'finished' callback is called in all error cases
			EOS_LobbySearch_Find(m_SearchHandle, &FindOptions, nullptr, [](const EOS_LobbySearch_FindCallbackInfo* Data)
				{
					if (Data->ResultCode == EOS_EResult::EOS_Success)
					{
						EOS_LobbySearch_GetSearchResultCountOptions searchResultOpts;
						searchResultOpts.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;

						auto searchHandle = NGMP_OnlineServicesManager::GetInstance()->m_SearchHandle;
						uint32_t numSessions = EOS_LobbySearch_GetSearchResultCount(searchHandle, &searchResultOpts);

						std::vector<NGMP_LobbyInfo> vecLobbies = std::vector<NGMP_LobbyInfo>();
						

						// TODO_NGMP: None of these EOS handles ever get free'd, anywhere

						if (numSessions == 0)
						{
							// TODO_NGMP: Give more info to play
						}
						else
						{
							// TODO_NGMP: Give more info to play
						}

						for (uint32_t i = 0; i < numSessions; ++i)
						{
							EOS_LobbySearch_CopySearchResultByIndexOptions getOpts;
							getOpts.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
							getOpts.LobbyIndex = i;

							EOS_HLobbyDetails sessionDetails = nullptr;
							EOS_LobbySearch_CopySearchResultByIndex(searchHandle, &getOpts, &sessionDetails);

							EOS_LobbyDetails_Info* SessionInfo = NULL;
							EOS_LobbyDetails_CopyInfoOptions CopyOptions = {};
							CopyOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
							EOS_EResult CopyResult = EOS_LobbyDetails_CopyInfo(sessionDetails, &CopyOptions, &SessionInfo);
							if (CopyResult == EOS_EResult::EOS_Success)
							{
								// TODO_NGMP: Filter more here, if its empty or is orphaned, dont bother returning it
								NGMP_LobbyInfo newEntry = NGMP_LobbyInfo();

								// parse owner name
								char szUserID[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
								int len = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
								EOS_EResult r = EOS_ProductUserId_ToString(SessionInfo->LobbyOwnerUserId, szUserID, &len);
								if (r != EOS_EResult::EOS_Success)
								{
									// TODO_NGMP: Error
									OutputDebugString("EOS error!\n");
								}

								newEntry.strLobbyOwner = AsciiString(szUserID);
								newEntry.strLobbyName = SessionInfo->LobbyId;
								newEntry.numMembers = SessionInfo->MaxMembers - SessionInfo->AvailableSlots;
								newEntry.maxMembers = SessionInfo->MaxMembers;

								

								vecLobbies.push_back(newEntry);

								// Free it up
								EOS_LobbyDetails_Info_Release(SessionInfo);
							}

							//EOS_Sessions_JoinSession

							EOS_LobbyDetails_Release(sessionDetails);
						}

						if (NGMP_OnlineServicesManager::GetInstance()->m_PendingSearchLobbyCompleteCallback != nullptr)
						{
							NGMP_OnlineServicesManager::GetInstance()->m_PendingSearchLobbyCompleteCallback(vecLobbies);
						}
					}
					else
					{
						OutputDebugString("Failed to search for Lobbies!");
					}
				});
		}
	}

	UnicodeString m_PendingCreation_LobbyName;
	void CreateLobby(UnicodeString strLobbyName)
	{
		m_PendingCreation_LobbyName = strLobbyName;

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

					// add version param
					EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

					EOS_Lobby_UpdateLobbyModificationOptions ModifyOptions = {};
					ModifyOptions.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
					ModifyOptions.LobbyId = Data->LobbyId;
					ModifyOptions.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetEOSUser();
					EOS_HLobbyModification LobbyModification = nullptr;
					EOS_EResult Result = EOS_Lobby_UpdateLobbyModification(LobbyHandle, &ModifyOptions, &LobbyModification);

					// TODO_NGMP: Handle result properly
					if (Result != EOS_EResult::EOS_Success)
					{
						OutputDebugString("Failed to set search param");
					}

					// VERSION
					{
						EOS_LobbyModification_AddAttributeOptions AddAttributeModOptions;
						EOS_Lobby_AttributeData AttributeData;
						AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
						AttributeData.Key = "VERSION";
						AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_INT64;
						AttributeData.Value.AsInt64 = 1337; // TODO_NGMP: Proper value

						AddAttributeModOptions.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
						AddAttributeModOptions.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
						AddAttributeModOptions.Attribute = &AttributeData;

						EOS_EResult AddResult = EOS_LobbyModification_AddAttribute(LobbyModification, &AddAttributeModOptions);
						if (AddResult != EOS_EResult::EOS_Success)
						{
							OutputDebugString("Failed to set lobby param");
						}
					}

					// NAME
					{
						EOS_LobbyModification_AddAttributeOptions AddAttributeModOptions;
						EOS_Lobby_AttributeData AttributeData;
						AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
						AttributeData.Key = "NAME";
						AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;

						AsciiString strLobbyName = AsciiString();
						strLobbyName.translate(NGMP_OnlineServicesManager::GetInstance()->m_PendingCreation_LobbyName);
						AttributeData.Value.AsUtf8 = strLobbyName.str();

						AddAttributeModOptions.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
						AddAttributeModOptions.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
						AddAttributeModOptions.Attribute = &AttributeData;

						EOS_EResult AddResult = EOS_LobbyModification_AddAttribute(LobbyModification, &AddAttributeModOptions);
						if (AddResult != EOS_EResult::EOS_Success)
						{
							OutputDebugString("Failed to set lobby param");
						}
					}

					// now update lobby
					EOS_Lobby_UpdateLobbyOptions UpdateOptions = {};
					UpdateOptions.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
					UpdateOptions.LobbyModificationHandle = LobbyModification;
					EOS_Lobby_UpdateLobby(LobbyHandle, &UpdateOptions, nullptr, [](const EOS_Lobby_UpdateLobbyCallbackInfo* Data)
						{
							if (Data->ResultCode == EOS_EResult::EOS_Success)
							{
								OutputDebugString("Lobby Updated!\n");
							}
							else
							{
								OutputDebugString("Lobby Update failed!\n");
							}
						});
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

	AsciiString GetDisplayName()
	{
		return AsciiString(m_strDisplayName.c_str());
	}


	//bool m_bPendingLoginFlag = false;

	std::vector<NetworkRoom> GetGroupRooms()
	{
		return m_vecRooms;
	}

	std::function<void(NGMP_ENATType previousNATType, NGMP_ENATType newNATType)> m_cbNATTypeChanged = nullptr;
	void RegisterForNATTypeChanges(std::function<void(NGMP_ENATType previousNATType, NGMP_ENATType newNATType)> cbNATTypeChanged) { m_cbNATTypeChanged = cbNATTypeChanged; }

	EOS_ProductUserId GetEOSUser() const { return m_EOSUserID; }

	void CacheNATType(NGMP_ENATType natType)
	{
		NGMP_ENATType oldNATType = m_NATType;
		
		m_NATType = natType;
		m_cbNATTypeChanged(oldNATType, m_NATType);
	}
	NGMP_ENATType GetNATType() const { return m_NATType; }
	AsciiString GetNATTypeString() const
	{
		switch (m_NATType)
		{
		default:
		case NGMP_ENATType::NAT_TYPE_UNDETERMINED:
			return AsciiString("Undetermined");

		case NGMP_ENATType::NAT_TYPE_OPEN:
			return AsciiString("Open");

		case NGMP_ENATType::NAT_TYPE_MODERATE:
			return AsciiString("Moderate");

		case NGMP_ENATType::NAT_TYPE_STRICT:
			return AsciiString("Strict");
		}

		return AsciiString("Undetermined");
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

	NGMP_ENATType m_NATType = NGMP_ENATType::NAT_TYPE_UNDETERMINED;
};