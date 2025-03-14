#include "GameNetwork/NextGenMP/NGMP_interfaces.h"

void NGMP_OnlineServices_RoomsInterface::JoinRoom(int roomIndex, std::function<void()> onStartCallback, std::function<void()> onCompleteCallback)
{
	// TODO_NGMP: Only set this when actually joining successfully
	m_CurrentRoomID = roomIndex;

	m_PendingRoomJoinCompleteCallback = onCompleteCallback;

	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	EOS_Lobby_CreateLobbySearchOptions CreateSearchOptions = {};
	CreateSearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
	CreateSearchOptions.MaxResults = 1;


	EOS_EResult Result = EOS_Lobby_CreateLobbySearch(LobbyHandle, &CreateSearchOptions, &m_SearchHandle);

	if (Result == EOS_EResult::EOS_Success)
	{
		EOS_LobbySearch_FindOptions FindOptions = {};
		FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
		FindOptions.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();

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

		// get room internal name
		// TODO_NGMP: Safer method here, plus handle case where not found
		NetworkRoom targetNetworkRoom = NGMP_OnlineServicesManager::GetInstance()->GetGroupRooms().at(roomIndex);

		// must match name
		EOS_LobbySearch_SetLobbyIdOptions lobbyIdOpts;
		lobbyIdOpts.ApiVersion = EOS_LOBBYSEARCH_SETLOBBYID_API_LATEST;
		lobbyIdOpts.LobbyId = targetNetworkRoom.GetRoomInternalName().str();

		EOS_EResult res = EOS_LobbySearch_SetLobbyId(m_SearchHandle, &lobbyIdOpts);
		if (res != EOS_EResult::EOS_Success)
		{
			OutputDebugString("Failed to set name search param");
		}
		
		if (onStartCallback != nullptr)
		{
			onStartCallback();
		}

		// TODO_NGMP: Make sure 'finished' callback is called in all error cases
		EOS_LobbySearch_Find(m_SearchHandle, &FindOptions, (void*)roomIndex, [](const EOS_LobbySearch_FindCallbackInfo* Data)
			{
				int roomIndex = (int)Data->ClientData;

				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					EOS_LobbySearch_GetSearchResultCountOptions searchResultOpts;
					searchResultOpts.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;

					auto searchHandle = NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_SearchHandle;
					uint32_t numSessions = EOS_LobbySearch_GetSearchResultCount(searchHandle, &searchResultOpts);

					// cache our details
					EOS_LobbySearch_CopySearchResultByIndexOptions getOpts;
					getOpts.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
					getOpts.LobbyIndex = 0;

					NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_currentRoomDetailsHandle = nullptr;
					EOS_LobbySearch_CopySearchResultByIndex(searchHandle, &getOpts, &(NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_currentRoomDetailsHandle));
					
					if (numSessions == 0)
					{
						NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->CreateRoom(roomIndex);
					}
					else
					{
						EOS_Lobby_JoinLobbyOptions joinOpts;
						joinOpts.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
						joinOpts.LobbyDetailsHandle = NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_currentRoomDetailsHandle;
						joinOpts.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
						joinOpts.bPresenceEnabled = false;
						joinOpts.LocalRTCOptions = nullptr;
						joinOpts.bCrossplayOptOut = false;
						joinOpts.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_ManualJoin;


						EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());
						EOS_Lobby_JoinLobby(LobbyHandle, &joinOpts, nullptr, [](const EOS_Lobby_JoinLobbyCallbackInfo* Data)
							{
								if (Data->ResultCode == EOS_EResult::EOS_Success)
								{
									OutputDebugString("Joined room!");
								}
								else
								{
									OutputDebugString("Failed to join room!");
								}
							});

						if (NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_PendingRoomJoinCompleteCallback != nullptr)
						{
							NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_PendingRoomJoinCompleteCallback();
						}
					}
				}
				else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
				{
					// create
					NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->CreateRoom(roomIndex);
				}
				else
				{
					OutputDebugString("Failed to search for Lobbies!");
				}
			});
	}
}

void NGMP_OnlineServices_RoomsInterface::CreateRoom(int roomIndex)
{
	NetworkRoom targetNetworkRoom = NGMP_OnlineServicesManager::GetInstance()->GetGroupRooms().at(roomIndex);

	EOS_HLobby lobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	EOS_Lobby_CreateLobbyOptions* createLobbyOpts = new EOS_Lobby_CreateLobbyOptions();
	createLobbyOpts->ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	createLobbyOpts->LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
	createLobbyOpts->MaxLobbyMembers = 64;
	createLobbyOpts->PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
	createLobbyOpts->bPresenceEnabled = false;
	createLobbyOpts->BucketId = "NETWORK_ROOMS";
	createLobbyOpts->LobbyId = targetNetworkRoom.GetRoomInternalName().str();
	createLobbyOpts->bDisableHostMigration = false; // we can host migrate netrooms
	createLobbyOpts->bEnableRTCRoom = false;
	createLobbyOpts->LocalRTCOptions = nullptr;
	createLobbyOpts->bEnableJoinById = true;
	createLobbyOpts->bRejoinAfterKickRequiresInvite = false;
	createLobbyOpts->AllowedPlatformIds = nullptr;
	createLobbyOpts->AllowedPlatformIdsCount = 0;
	createLobbyOpts->bCrossplayOptOut = false;
	createLobbyOpts->RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_ManualJoin;

	EOS_Lobby_CreateLobby(lobbyHandle, createLobbyOpts, (void*)roomIndex, [](const EOS_Lobby_CreateLobbyCallbackInfo* Data)
		{
			int roomIndex = (int)Data->ClientData;

			// TODO_NGMP: If we get a result saying it exists, just join it
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				// TODO_NGMP: We need to search again to get lobby details handle... but we shouldnt just call join again, its hacky
				NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->JoinRoom(roomIndex, nullptr, nullptr);

				OutputDebugString("Lobby created!\n");

				if (NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_PendingRoomJoinCompleteCallback != nullptr)
				{
					NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_PendingRoomJoinCompleteCallback();
				}
				// trigger join callback
			}
			else
			{
				OutputDebugString("Failed to create lobby!\n");
			}
		});
}

std::vector<NetworkRoomMember> NGMP_OnlineServices_RoomsInterface::GetMembersListForCurrentRoom()
{
	if (m_CurrentRoomID != -1)
	{
		// TODO_NGMP: Cache this and respond to join/leave events instead
		std::vector<NetworkRoomMember> vecMembers = std::vector<NetworkRoomMember>();

		EOS_LobbyDetails_GetMemberCountOptions memberCountOpts;
		memberCountOpts.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;

		int numMembers = EOS_LobbyDetails_GetMemberCount(NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_currentRoomDetailsHandle, &memberCountOpts);
		for (int i = 0; i < numMembers; ++i)
		{
			EOS_LobbyDetails_GetMemberByIndexOptions getMemberOpts;
			getMemberOpts.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
			getMemberOpts.MemberIndex = i;

			EOS_ProductUserId roomMemberEOSID = EOS_LobbyDetails_GetMemberByIndex(NGMP_OnlineServicesManager::GetInstance()->GetRoomsInterface()->m_currentRoomDetailsHandle, &getMemberOpts);

			char szUserID[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
			int len = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
			EOS_EResult r = EOS_ProductUserId_ToString(roomMemberEOSID, szUserID, &len);
			if (r != EOS_EResult::EOS_Success)
			{
				// TODO_NGMP: Error
				OutputDebugString("EOS error!\n");
			}
			NetworkRoomMember roomMember;
			roomMember.m_strName = AsciiString(szUserID);
		}

		return vecMembers;
	}

	return std::vector<NetworkRoomMember>();
}

/*
void NGMP_OnlineServices_RoomsInterface::CreateLobby(UnicodeString strLobbyName)
{
	m_PendingCreation_LobbyName = strLobbyName;

	// TODO_NGMP: Correct values
	
}
*/