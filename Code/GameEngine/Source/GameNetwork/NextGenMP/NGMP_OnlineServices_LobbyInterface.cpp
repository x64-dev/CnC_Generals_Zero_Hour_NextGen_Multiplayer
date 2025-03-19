#include "GameNetwork/NextGenMP/NGMP_interfaces.h"
#include "GameNetwork/NextGenMP/Packets/NetworkPacket_NetRoom_Hello.h"
#include "GameNetwork/NextGenMP/Packets/NetworkPacket_NetRoom_HelloAck.h"
#include "GameNetwork/NextGenMP/Packets/NetworkPacket_NetRoom_ChatMessage.h"

UnicodeString NGMP_OnlineServices_LobbyInterface::GetCurrentLobbyDisplayName()
{
	UnicodeString strDisplayName;
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	// get a handle to our lobby
	EOS_Lobby_CopyLobbyDetailsHandleOptions opts;
	opts.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLE_API_LATEST;
	opts.LobbyId = m_strCurrentLobbyID.c_str();
	opts.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();

	EOS_HLobbyDetails LobbyInstHandle;
	EOS_EResult getLobbyHandlResult = EOS_Lobby_CopyLobbyDetailsHandle(LobbyHandle, &opts, &LobbyInstHandle);

	if (getLobbyHandlResult == EOS_EResult::EOS_Success)
	{
		EOS_Lobby_Attribute* attr = nullptr;
		EOS_LobbyDetails_CopyAttributeByKeyOptions copyAttrOpts;
		copyAttrOpts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYKEY_API_LATEST;
		copyAttrOpts.AttrKey = "NAME";
		EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyAttributeByKey(LobbyInstHandle, &copyAttrOpts, &attr);
		if (rCopyMemberAttr == EOS_EResult::EOS_Success)
		{
			// TODO_NGMP: Validate type too, could cause a crash
			strDisplayName.format(L"%hs", attr->Data->Value.AsUtf8);
		}
	}
	
	if (strDisplayName.isEmpty())
	{
		strDisplayName.format(L"%hs", m_strCurrentLobbyID);
	}
	

	return strDisplayName;
}

void NGMP_OnlineServices_LobbyInterface::SendChatMessageToCurrentLobby(UnicodeString& strChatMsgUnicode)
{
	// TODO_NGMP: Support unicode again
	AsciiString strChatMsg;
	strChatMsg.translate(strChatMsgUnicode);

	NetRoom_ChatMessagePacket chatPacket(strChatMsg);

	std::vector<EOS_ProductUserId> vecUsers;
	for (auto kvPair : m_mapMembers)
	{
		vecUsers.push_back(kvPair.first);
	}

	m_lobbyMesh.SendToMesh(chatPacket, vecUsers);
}

NGMP_OnlineServices_LobbyInterface::NGMP_OnlineServices_LobbyInterface()
{
	// Register for EOS callbacks, we will handle them internally and pass them onto the game as necessary
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	// player joined/left etc
	EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions lobbyStatusRecievedOpts;
	lobbyStatusRecievedOpts.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYMEMBERSTATUSRECEIVED_API_LATEST;
	EOS_NotificationId notID1 = EOS_Lobby_AddNotifyLobbyMemberStatusReceived(LobbyHandle, &lobbyStatusRecievedOpts, nullptr, [](const EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo* Data)
		{
			NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->UpdateRoomDataCache();
		});

	// player data changed
	EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions lobbyMemberUpdateRecievedOpts;
	lobbyMemberUpdateRecievedOpts.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYMEMBERUPDATERECEIVED_API_LATEST;
	EOS_NotificationId notID2 = EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(LobbyHandle, &lobbyMemberUpdateRecievedOpts, nullptr, [](const EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo* Data)
		{
			NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->UpdateRoomDataCache();
		});

	notID1 = notID1;
	notID2 = notID2;
}

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
							newEntry.strLobbyOwnerID = AsciiString(szUserID);

							// display name
							{
								// TODO_NGMP: Handle missing lobby properties

								EOS_Lobby_Attribute* attr = nullptr;
								EOS_LobbyDetails_CopyAttributeByKeyOptions copyAttrOpts;
								copyAttrOpts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYKEY_API_LATEST;
								copyAttrOpts.AttrKey = "NAME";
								EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyAttributeByKey(sessionDetails, &copyAttrOpts, &attr);
								if (rCopyMemberAttr == EOS_EResult::EOS_Success)
								{
									// TODO_NGMP: Validate type too, could cause a crash
									newEntry.strLobbyName = attr->Data->Value.AsUtf8;
								}
								else
								{
									// TODO_NGMP: Maybe dont return these until it resolves?
									newEntry.strLobbyName = SessionInfo->LobbyId;
								}
							}
							
							// owner name
							{
								// TODO_NGMP: Handle missing lobby properties

								EOS_Lobby_Attribute* attr = nullptr;
								EOS_LobbyDetails_CopyAttributeByKeyOptions copyAttrOpts;
								copyAttrOpts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYKEY_API_LATEST;
								copyAttrOpts.AttrKey = "OWNER_NAME";
								EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyAttributeByKey(sessionDetails, &copyAttrOpts, &attr);
								if (rCopyMemberAttr == EOS_EResult::EOS_Success)
								{
									// TODO_NGMP: Validate type too, could cause a crash
									newEntry.strLobbyOwnerName = attr->Data->Value.AsUtf8;
								}
								else
								{
									// TODO_NGMP: Maybe dont return these until it resolves?
									newEntry.strLobbyOwnerName = newEntry.strLobbyOwnerID;
								}
							}

							// map name
							{
								// TODO_NGMP: Handle missing lobby properties

								EOS_Lobby_Attribute* attr = nullptr;
								EOS_LobbyDetails_CopyAttributeByKeyOptions copyAttrOpts;
								copyAttrOpts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYKEY_API_LATEST;
								copyAttrOpts.AttrKey = "MAP";
								EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyAttributeByKey(sessionDetails, &copyAttrOpts, &attr);
								if (rCopyMemberAttr == EOS_EResult::EOS_Success)
								{
									// TODO_NGMP: Validate type too, could cause a crash
									newEntry.strMapName = attr->Data->Value.AsUtf8;
								}
								else
								{
									// TODO_NGMP: Maybe dont return these until it resolves?
									newEntry.strMapName = SessionInfo->LobbyId;
								}
							}

							// NAT
							{
								// TODO_NGMP: Handle missing lobby properties

								EOS_Lobby_Attribute* attr = nullptr;
								EOS_LobbyDetails_CopyAttributeByKeyOptions copyAttrOpts;
								copyAttrOpts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYKEY_API_LATEST;
								copyAttrOpts.AttrKey = "NAT";
								EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyAttributeByKey(sessionDetails, &copyAttrOpts, &attr);
								if (rCopyMemberAttr == EOS_EResult::EOS_Success)
								{
									// TODO_NGMP: Validate type too, could cause a crash
									newEntry.NATType = (NGMP_ENATType)attr->Data->Value.AsInt64;
								}
								else
								{
									// TODO_NGMP: Maybe dont return these until it resolves?
									newEntry.NATType = NGMP_ENATType::NAT_TYPE_UNDETERMINED;
								}
							}
							
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

bool NGMP_OnlineServices_LobbyInterface::IsHost()
{
	if (!m_strCurrentLobbyID.empty())
	{
		EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

		// Get handle to current lobby
		EOS_Lobby_CopyLobbyDetailsHandleOptions opts;
		opts.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLE_API_LATEST;
		opts.LobbyId = m_strCurrentLobbyID.c_str();
		opts.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
		EOS_HLobbyDetails LobbyInstHandle = nullptr;
		EOS_EResult getLobbyHandlResult = EOS_Lobby_CopyLobbyDetailsHandle(LobbyHandle, &opts, &LobbyInstHandle);

		if (getLobbyHandlResult == EOS_EResult::EOS_Success && LobbyInstHandle != nullptr)
		{
			EOS_LobbyDetails_GetLobbyOwnerOptions getLobbyOwnerOpts;
			getLobbyOwnerOpts.ApiVersion = EOS_LOBBYDETAILS_GETLOBBYOWNER_API_LATEST;

			EOS_ProductUserId currentLobbyHost = EOS_LobbyDetails_GetLobbyOwner(LobbyInstHandle, &getLobbyOwnerOpts);
			return currentLobbyHost == NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
		}
		else
		{
			return false;
		}
	}

	return false;
}

void NGMP_OnlineServices_LobbyInterface::ApplyLocalUserPropertiesToCurrentNetworkRoom()
{
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	EOS_Lobby_UpdateLobbyModificationOptions ModifyOptions = {};
	ModifyOptions.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
	ModifyOptions.LobbyId = m_strCurrentLobbyID.c_str();
	ModifyOptions.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
	EOS_HLobbyModification LobbyModification = nullptr;
	EOS_EResult Result = EOS_Lobby_UpdateLobbyModification(LobbyHandle, &ModifyOptions, &LobbyModification);
	if (Result != EOS_EResult::EOS_Success)
	{
		// TODO_NGMP: Error
		NetworkLog("[NGMP] Failed to EOS_Lobby_UpdateLobbyModification!\n");
	}

	EOS_Lobby_AttributeData AttributeData;
	AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
	AttributeData.Key = "DISPLAY_NAME";
	AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;
	AttributeData.Value.AsUtf8 = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetDisplayName().str();

	EOS_LobbyModification_AddMemberAttributeOptions addMemberAttrOpts;
	addMemberAttrOpts.ApiVersion = EOS_LOBBYMODIFICATION_ADDMEMBERATTRIBUTE_API_LATEST;
	addMemberAttrOpts.Attribute = &AttributeData;
	addMemberAttrOpts.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
	EOS_EResult r = EOS_LobbyModification_AddMemberAttribute(LobbyModification, &addMemberAttrOpts);
	if (r != EOS_EResult::EOS_Success)
	{
		// TODO_NGMP: Error
		NetworkLog("[NGMP] Failed to set our local user properties in net room!\n");
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

	// join the network mesh too
	m_lobbyMesh.ConnectToMesh(m_strCurrentLobbyID.c_str());

	if (m_RosterNeedsRefreshCallback != nullptr)
	{
		m_RosterNeedsRefreshCallback();
	}
}

void NGMP_OnlineServices_LobbyInterface::UpdateRoomDataCache()
{
	// process users
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());
	
	// get a handle to our lobby
	EOS_Lobby_CopyLobbyDetailsHandleOptions opts;
	opts.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLE_API_LATEST;
	opts.LobbyId = m_strCurrentLobbyID.c_str();
	opts.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();

	EOS_HLobbyDetails LobbyInstHandle;
	EOS_EResult getLobbyHandlResult = EOS_Lobby_CopyLobbyDetailsHandle(LobbyHandle, &opts, &LobbyInstHandle);

	if (getLobbyHandlResult == EOS_EResult::EOS_Success)
	{
		// get each member
		EOS_LobbyDetails_GetMemberCountOptions optsMemberCount;
		optsMemberCount.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;

		uint32_t numMembers = EOS_LobbyDetails_GetMemberCount(LobbyInstHandle, &optsMemberCount);
		NetworkLog("[NGMP] Network room has %d members", numMembers);

		for (uint32_t memberIndex = 0; memberIndex < numMembers; ++memberIndex)
		{
			EOS_LobbyDetails_GetMemberByIndexOptions opts4;
			opts4.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
			opts4.MemberIndex = memberIndex;
			EOS_ProductUserId lobbyMember = EOS_LobbyDetails_GetMemberByIndex(LobbyInstHandle, &opts4);

			char szUserID[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
			int len = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
			EOS_EResult r = EOS_ProductUserId_ToString(lobbyMember, szUserID, &len);
			if (r != EOS_EResult::EOS_Success)
			{
				// TODO_NGMP: Error
				NetworkLog("[NGMP] EOS error!\n");
			}

			NetworkLog("[NGMP] Network room member %d is %s", memberIndex, szUserID);

			// does the user already exist? if not, register the user
			if (m_mapMembers.find(lobbyMember) == m_mapMembers.end())
			{
				m_mapMembers.emplace(lobbyMember, LobbyMember());

				// new member, send them a hello!
				// TODO_NGMP: More robust impl
				m_lobbyMesh.SendHelloMsg(lobbyMember);
			}

			// read member data we care about
			EOS_Lobby_Attribute* attrDisplayName = nullptr;
			EOS_LobbyDetails_CopyMemberAttributeByKeyOptions copyAttrOpts;
			copyAttrOpts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYKEY_API_LATEST;
			copyAttrOpts.AttrKey = "DISPLAY_NAME";
			copyAttrOpts.TargetUserId = lobbyMember;
			EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyMemberAttributeByKey(LobbyInstHandle, &copyAttrOpts, &attrDisplayName);
			if (rCopyMemberAttr != EOS_EResult::EOS_Success)
			{
				// Handle gracefully and fall back to stringified version of user id
				m_mapMembers[lobbyMember].m_strName = AsciiString(szUserID);
				NetworkLog("[NGMP] Couldn't read display name\n");
			}
			else
			{
				m_mapMembers[lobbyMember].m_strName = attrDisplayName->Data->Value.AsUtf8;
			}

		}
	}

	if (m_RosterNeedsRefreshCallback != nullptr)
	{
		m_RosterNeedsRefreshCallback();
	}
}

void NGMP_OnlineServices_LobbyInterface::CreateLobby(UnicodeString strLobbyName, UnicodeString strInitialMap, int initialMaxSize)
{
	m_PendingCreation_LobbyName = strLobbyName;
	m_PendingCreation_InitialMap = strInitialMap;

	// TODO_NGMP: Correct values
	EOS_HLobby lobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	EOS_Lobby_CreateLobbyOptions* createLobbyOpts = new EOS_Lobby_CreateLobbyOptions();
	createLobbyOpts->ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	createLobbyOpts->LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
	createLobbyOpts->MaxLobbyMembers = initialMaxSize;
	createLobbyOpts->PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
	createLobbyOpts->bPresenceEnabled = false;
	createLobbyOpts->BucketId = "CUSTOM_MATCH";
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
				NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->SetCurrentLobbyID(Data->LobbyId);
				
				NetworkLog("[NGMP] Lobby created with ID %s!\n", Data->LobbyId);

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

				// OWNER NAME
				{
					EOS_LobbyModification_AddAttributeOptions AddAttributeModOptions;
					EOS_Lobby_AttributeData AttributeData;
					AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
					AttributeData.Key = "OWNER_NAME";
					AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;

					AttributeData.Value.AsUtf8 = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetDisplayName().str();

					AddAttributeModOptions.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
					AddAttributeModOptions.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
					AddAttributeModOptions.Attribute = &AttributeData;

					EOS_EResult AddResult = EOS_LobbyModification_AddAttribute(LobbyModification, &AddAttributeModOptions);
					if (AddResult != EOS_EResult::EOS_Success)
					{
						NetworkLog("[NGMP] Failed to set lobby param");
					}
				}

				// MAP
				{
					EOS_LobbyModification_AddAttributeOptions AddAttributeModOptions;
					EOS_Lobby_AttributeData AttributeData;
					AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
					AttributeData.Key = "MAP";
					AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;
;
					AsciiString strMapName = AsciiString();
					strMapName.translate(NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingCreation_InitialMap);
					AttributeData.Value.AsUtf8 = strMapName.str();

					AddAttributeModOptions.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
					AddAttributeModOptions.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
					AddAttributeModOptions.Attribute = &AttributeData;

					EOS_EResult AddResult = EOS_LobbyModification_AddAttribute(LobbyModification, &AddAttributeModOptions);
					if (AddResult != EOS_EResult::EOS_Success)
					{
						NetworkLog("[NGMP] Failed to set lobby param");
					}
				}

				// NAT TYPE
				{
					EOS_LobbyModification_AddAttributeOptions AddAttributeModOptions;
					EOS_Lobby_AttributeData AttributeData;
					AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
					AttributeData.Key = "NAT";
					AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_INT64;
					;
					AttributeData.Value.AsInt64 = NGMP_OnlineServicesManager::GetInstance()->GetNATType();

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

			// Set our properties
			NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->ResetCachedRoomData();
			NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->ApplyLocalUserPropertiesToCurrentNetworkRoom();
		});
}

void NGMP_OnlineServices_LobbyMesh::SendHelloMsg(EOS_ProductUserId targetUser)
{
	NetRoom_HelloPacket helloPacket;

	std::vector<EOS_ProductUserId> vecUsers;
	vecUsers.push_back(targetUser);

	SendToMesh(helloPacket, vecUsers);
}

void NGMP_OnlineServices_LobbyMesh::SendHelloAckMsg(EOS_ProductUserId targetUser)
{
	NetRoom_HelloAckPacket helloAckPacket;

	std::vector<EOS_ProductUserId> vecUsers;
	vecUsers.push_back(targetUser);

	SendToMesh(helloAckPacket, vecUsers);
}

void NGMP_OnlineServices_LobbyMesh::SendToMesh(NetworkPacket& packet, std::vector<EOS_ProductUserId> vecTargetUsers)
{
	auto P2PHandle = EOS_Platform_GetP2PInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	CBitStream* pBitStream = packet.Serialize();

	for (EOS_ProductUserId targetUser : vecTargetUsers)
	{
		EOS_P2P_SendPacketOptions sendPacketOptions;
		sendPacketOptions.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
		sendPacketOptions.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
		sendPacketOptions.RemoteUserId = targetUser;
		sendPacketOptions.SocketId = &m_SockID;
		sendPacketOptions.Channel = 2;
		sendPacketOptions.DataLengthBytes = (uint32_t)pBitStream->GetNumBytesUsed();
		sendPacketOptions.Data = (void*)pBitStream->GetRawBuffer();
		sendPacketOptions.bAllowDelayedDelivery = true;
		sendPacketOptions.Reliability = EOS_EPacketReliability::EOS_PR_ReliableOrdered;
		sendPacketOptions.bDisableAutoAcceptConnection = false;

		// TODO_NGMP: Support more packet types obviously

		EOS_EResult result = EOS_P2P_SendPacket(P2PHandle, &sendPacketOptions);

		char szEOSUserID[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
		int32_t outLenLocal = sizeof(szEOSUserID);
		EOS_ProductUserId_ToString(targetUser, szEOSUserID, &outLenLocal);
		NetworkLog("[NGMP]: Sending Packet with %d bytes to %s with result %d", pBitStream->GetNumBytesUsed(), szEOSUserID, result);
	}
}

void NGMP_OnlineServices_LobbyMesh::ConnectToMesh(const char* szRoomID)
{
	m_SockID.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
	strcpy(m_SockID.SocketName, szRoomID);

	// TODO_NGMP: Dont automatically accept connections that aren't in the room roster, security

	// connection created callback
	auto P2PHandle = EOS_Platform_GetP2PInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	// TODO_NGMP: This is specific to socket ID, unregister it when we leave or join another room
	EOS_P2P_AddNotifyPeerConnectionEstablishedOptions opts;
	opts.ApiVersion = EOS_P2P_ADDNOTIFYPEERCONNECTIONESTABLISHED_API_LATEST;
	opts.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
	opts.SocketId = &m_SockID;
	EOS_P2P_AddNotifyPeerConnectionEstablished(P2PHandle, &opts, nullptr, [](const EOS_P2P_OnPeerConnectionEstablishedInfo* Data)
		{
			LobbyMember* pMember = NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->GetRoomMemberFromID(Data->RemoteUserId);

			if (pMember != nullptr)
			{
				// TODO_NGMP: Handle reconnection
				if (Data->NetworkType == EOS_ENetworkConnectionType::EOS_NCT_NoConnection)
				{
					pMember->m_connectionState = ENetworkRoomMemberConnectionState::NOT_CONNECTED;
				}
				else if (Data->NetworkType == EOS_ENetworkConnectionType::EOS_NCT_DirectConnection)
				{
					pMember->m_connectionState = ENetworkRoomMemberConnectionState::CONNECTED_DIRECT;
				}
				else if (Data->NetworkType == EOS_ENetworkConnectionType::EOS_NCT_RelayedConnection)
				{
					pMember->m_connectionState = ENetworkRoomMemberConnectionState::CONNECTED_RELAYED;
				}
			}

			// invoke a roster change so the UI updates
			if (NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_RosterNeedsRefreshCallback != nullptr)
			{
				NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_RosterNeedsRefreshCallback();
			}
		});
}

void NGMP_OnlineServices_LobbyMesh::Tick()
{
	auto P2PHandle = EOS_Platform_GetP2PInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	uint8_t channelToUse = (uint8_t)2;

	// recv
	EOS_P2P_GetNextReceivedPacketSizeOptions sizeOptions;
	sizeOptions.ApiVersion = EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST;
	sizeOptions.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
	sizeOptions.RequestedChannel = &channelToUse; // room mesh channels

	uint32_t numBytes = 0;
	while (EOS_P2P_GetNextReceivedPacketSize(P2PHandle, &sizeOptions, &numBytes) == EOS_EResult::EOS_Success)
	{
		CBitStream bitstream(numBytes);

		EOS_P2P_ReceivePacketOptions options;
		options.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
		options.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();
		options.MaxDataSizeBytes = numBytes;
		options.RequestedChannel = &channelToUse; // room mesh channels

		EOS_ProductUserId outRemotePeerID = nullptr;
		EOS_P2P_SocketId outSocketID;
		uint8_t outChannel = 0;
		EOS_EResult result = EOS_P2P_ReceivePacket(P2PHandle, &options, &outRemotePeerID, &outSocketID, &outChannel, (void*)bitstream.GetRawBuffer(), &numBytes);

		if (result == EOS_EResult::EOS_Success)
		{
			// TODO_NGMP: Reject any packets from members not in the room? or mesh
			char szEOSUserID[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
			int32_t outLenLocal = sizeof(szEOSUserID);
			EOS_ProductUserId_ToString(outRemotePeerID, szEOSUserID, &outLenLocal);
			NetworkLog("[NGMP]: Received %d bytes from user %s", numBytes, szEOSUserID);

			EPacketID packetID = bitstream.Read<EPacketID>();

			if (packetID == EPacketID::PACKET_ID_NET_ROOM_HELLO)
			{
				NetRoom_HelloPacket helloPacket(bitstream);

				NetworkLog("[NGMP]: Got hello from %s, sending ack", szEOSUserID);
				SendHelloAckMsg(outRemotePeerID);
			}
			else if (packetID == EPacketID::PACKET_ID_NET_ROOM_HELLO_ACK)
			{
				NetRoom_HelloAckPacket helloAckPacket(bitstream);

				NetworkLog("[NGMP]: Received ack from %s, we're now connected", szEOSUserID);
			}
			else if (packetID == EPacketID::PACKET_ID_NET_ROOM_CHAT_MSG)
			{
				NetRoom_ChatMessagePacket chatPacket(bitstream);

				// TODO_NGMP: Support longer msgs
				NetworkLog("[NGMP]: Received chat message of len %d: %s", chatPacket.GetMsg().length(), chatPacket.GetMsg().c_str());

				// determine the username
				std::map<EOS_ProductUserId, LobbyMember>& mapRoomMembers = NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->GetMembersListForCurrentRoom();

				if (mapRoomMembers.find(outRemotePeerID) != mapRoomMembers.end())
				{
					UnicodeString str;
					str.format(L"%hs: %hs", mapRoomMembers[outRemotePeerID].m_strName.str(), chatPacket.GetMsg().c_str());
					NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_OnChatCallback(str);
				}
				else
				{
					// TODO_NGMP: Error, user sending us messages isnt in the room
				}
			}
		}
	}
}
