#include "GameNetwork/NextGenMP/NGMP_interfaces.h"

void NGMP_OnlineServices_LobbyInterface::SearchForLobbies(std::function<void()> onStartCallback, std::function<void(std::vector<NGMP_LobbyInfo>)> onCompleteCallback)
{
	m_PendingSearchLobbyCompleteCallback = onCompleteCallback;

	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	EOS_Lobby_CreateLobbySearchOptions CreateSearchOptions = {};
	CreateSearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
	CreateSearchOptions.MaxResults = 200;


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
				NetworkLog("[NGMP] Failed to set search param");
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

					auto searchHandle = NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_SearchHandle;
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
								NetworkLog("[NGMP] EOS error!\n");
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

					if (NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingSearchLobbyCompleteCallback != nullptr)
					{
						NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingSearchLobbyCompleteCallback(vecLobbies);
					}
				}
				else
				{
					NetworkLog("[NGMP] Failed to search for Lobbies!");
				}
			});
	}
}

void NGMP_OnlineServices_LobbyInterface::CreateLobby(UnicodeString strLobbyName)
{
	m_PendingCreation_LobbyName = strLobbyName;

	// TODO_NGMP: Correct values
	EOS_HLobby lobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	EOS_Lobby_CreateLobbyOptions* createLobbyOpts = new EOS_Lobby_CreateLobbyOptions();
	createLobbyOpts->ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	createLobbyOpts->LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
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
				NetworkLog("[NGMP] Lobby created!\n");

				// add version param
				EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

				EOS_Lobby_UpdateLobbyModificationOptions ModifyOptions = {};
				ModifyOptions.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
				ModifyOptions.LobbyId = Data->LobbyId;
				ModifyOptions.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
				EOS_HLobbyModification LobbyModification = nullptr;
				EOS_EResult Result = EOS_Lobby_UpdateLobbyModification(LobbyHandle, &ModifyOptions, &LobbyModification);

				// TODO_NGMP: Handle result properly
				if (Result != EOS_EResult::EOS_Success)
				{
					NetworkLog("[NGMP] Failed to set search param");
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
						NetworkLog("[NGMP] Failed to set lobby param");
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
					strLobbyName.translate(NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingCreation_LobbyName);
					AttributeData.Value.AsUtf8 = strLobbyName.str();

					AddAttributeModOptions.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
					AddAttributeModOptions.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
					AddAttributeModOptions.Attribute = &AttributeData;

					EOS_EResult AddResult = EOS_LobbyModification_AddAttribute(LobbyModification, &AddAttributeModOptions);
					if (AddResult != EOS_EResult::EOS_Success)
					{
						NetworkLog("[NGMP] Failed to set lobby param");
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
							NetworkLog("[NGMP] Lobby Updated!\n");
						}
						else
						{
							NetworkLog("[NGMP] Lobby Update failed!\n");
						}
					});
			}
			else
			{
				NetworkLog("[NGMP] Failed to create lobby!\n");
			}

			// TODO_NGMP: Impl
			NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->InvokeCreateLobbyCallback(Data->ResultCode == EOS_EResult::EOS_Success);
		});
}
