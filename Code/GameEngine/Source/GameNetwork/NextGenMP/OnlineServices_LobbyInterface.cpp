#include "GameNetwork/NextGenMP/NGMP_interfaces.h"
#include "GameNetwork/NextGenMP/Packets/NetworkPacket_NetRoom_Hello.h"
#include "GameNetwork/NextGenMP/Packets/NetworkPacket_NetRoom_HelloAck.h"
#include "GameNetwork/NextGenMP/Packets/NetworkPacket_NetRoom_ChatMessage.h"

extern NGMPGame* TheNGMPGame;

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

UnicodeString NGMP_OnlineServices_LobbyInterface::GetCurrentLobbyMapDisplayName()
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
		copyAttrOpts.AttrKey = "MAP_NAME";
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

AsciiString NGMP_OnlineServices_LobbyInterface::GetCurrentLobbyMapPath()
{
	AsciiString strPath;
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
		copyAttrOpts.AttrKey = "MAP_PATH";
		EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyAttributeByKey(LobbyInstHandle, &copyAttrOpts, &attr);
		if (rCopyMemberAttr == EOS_EResult::EOS_Success)
		{
			// TODO_NGMP: Validate type too, could cause a crash
			strPath = attr->Data->Value.AsUtf8;
		}
	}

	if (strPath.isEmpty())
	{
		strPath = m_strCurrentLobbyID.c_str();
	}

	return strPath;
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

	if (m_pLobbyMesh != nullptr)
	{
		m_pLobbyMesh->SendToMesh(chatPacket, vecUsers);
	}
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
	m_vecLobbies.clear();
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

					NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_vecLobbies = std::vector<NGMP_LobbyInfo>();


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

							// store lobby id
							newEntry.m_strLobbyID = SessionInfo->LobbyId;

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
								copyAttrOpts.AttrKey = "MAP_NAME";
								EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyAttributeByKey(sessionDetails, &copyAttrOpts, &attr);
								if (rCopyMemberAttr == EOS_EResult::EOS_Success)
								{
									// TODO_NGMP: Validate type too, could cause a crash
									newEntry.strMapDisplayName = attr->Data->Value.AsUtf8;
								}
								else
								{
									// TODO_NGMP: Maybe dont return these until it resolves?
									newEntry.strMapDisplayName = SessionInfo->LobbyId;
								}
							}

							// map path
							{
								// TODO_NGMP: Handle missing lobby properties

								EOS_Lobby_Attribute* attr = nullptr;
								EOS_LobbyDetails_CopyAttributeByKeyOptions copyAttrOpts;
								copyAttrOpts.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYKEY_API_LATEST;
								copyAttrOpts.AttrKey = "MAP_PATH";
								EOS_EResult rCopyMemberAttr = EOS_LobbyDetails_CopyAttributeByKey(sessionDetails, &copyAttrOpts, &attr);
								if (rCopyMemberAttr == EOS_EResult::EOS_Success)
								{
									// TODO_NGMP: Validate type too, could cause a crash
									newEntry.strMapPath = attr->Data->Value.AsUtf8;
								}
								else
								{
									// TODO_NGMP: Maybe dont return these until it resolves?
									newEntry.strMapPath = SessionInfo->LobbyId;
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



							NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_vecLobbies.push_back(newEntry);

							// Free it up
							EOS_LobbyDetails_Info_Release(SessionInfo);
						}

						//EOS_Sessions_JoinSession

						EOS_LobbyDetails_Release(sessionDetails);
					}

					if (NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingSearchLobbyCompleteCallback != nullptr)
					{
						NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingSearchLobbyCompleteCallback(NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_vecLobbies);
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
	m_pLobbyMesh = new NetworkMesh(ENetworkMeshType::GAME_LOBBY);
	m_pLobbyMesh->ConnectToMesh(m_strCurrentLobbyID.c_str());

	if (m_RosterNeedsRefreshCallback != nullptr)
	{
		m_RosterNeedsRefreshCallback();
	}

	// inform game instance too
	if (TheNGMPGame != nullptr)
	{
		TheNGMPGame->UpdateSlotsFromCurrentLobby();
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
		// get owner
		EOS_LobbyDetails_GetLobbyOwnerOptions getLobbyOwnerOpts;
		getLobbyOwnerOpts.ApiVersion = EOS_LOBBYDETAILS_GETLOBBYOWNER_API_LATEST;
		EOS_ProductUserId currentLobbyHost = EOS_LobbyDetails_GetLobbyOwner(LobbyInstHandle, &getLobbyOwnerOpts);

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
				m_mapMembers.emplace(lobbyMember, new LobbyMember());

				// new member, send them a hello!
				// TODO_NGMP: More robust impl
				m_pLobbyMesh->SendHelloMsg(lobbyMember);
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
				m_mapMembers[lobbyMember]->m_strName = AsciiString(szUserID);
				NetworkLog("[NGMP] Couldn't read display name\n");
			}
			else
			{
				m_mapMembers[lobbyMember]->m_strName = attrDisplayName->Data->Value.AsUtf8;
			}

			m_mapMembers[lobbyMember]->m_bIsHost = currentLobbyHost == lobbyMember;
		}
	}

	if (m_RosterNeedsRefreshCallback != nullptr)
	{
		m_RosterNeedsRefreshCallback();
	}

	// inform game instance too
	if (TheNGMPGame != nullptr)
	{
		TheNGMPGame->UpdateSlotsFromCurrentLobby();
	}
}

void NGMP_OnlineServices_LobbyInterface::JoinLobby(int index)
{
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());


	// We need to browse again for the lobby, EOS_Lobby_CopyLobbyDetailsHandle is only available to people in the lobby, so we need to use EOS_LobbySearch_CopySearchResultByIndex
	EOS_Lobby_CreateLobbySearchOptions CreateSearchOptions = {};
	CreateSearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
	CreateSearchOptions.MaxResults = 1;


	EOS_EResult Result = EOS_Lobby_CreateLobbySearch(LobbyHandle, &CreateSearchOptions, &m_SearchHandle);

	if (Result == EOS_EResult::EOS_Success)
	{
		EOS_LobbySearch_FindOptions FindOptions = {};
		FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
		FindOptions.LocalUserId = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetEOSUser();

		// TODO_NGMP: Safer method here, plus handle case where not found
		NGMP_LobbyInfo lobbyInfo = NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->GetLobbyFromIndex(index);

		// must match name
		EOS_LobbySearch_SetLobbyIdOptions lobbyIdOpts;
		lobbyIdOpts.ApiVersion = EOS_LOBBYSEARCH_SETLOBBYID_API_LATEST;
		lobbyIdOpts.LobbyId = lobbyInfo.m_strLobbyID.c_str();

		EOS_EResult res = EOS_LobbySearch_SetLobbyId(m_SearchHandle, &lobbyIdOpts);
		if (res != EOS_EResult::EOS_Success)
		{
			NetworkLog("[NGMP] Failed to set name search param");
		}

		// TODO_NGMP: Make sure 'finished' callback is called in all error cases
		EOS_LobbySearch_Find(m_SearchHandle, &FindOptions, (void*)index, [](const EOS_LobbySearch_FindCallbackInfo* Data)
			{
				//int lobbyIndex = (int)Data->ClientData;

				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					EOS_LobbySearch_GetSearchResultCountOptions searchResultOpts;
					searchResultOpts.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;

					auto searchHandle = NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_SearchHandle;
					uint32_t numSessions = EOS_LobbySearch_GetSearchResultCount(searchHandle, &searchResultOpts);

					// cache our details
					EOS_LobbySearch_CopySearchResultByIndexOptions getOpts;
					getOpts.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
					getOpts.LobbyIndex = 0;

					EOS_HLobbyDetails detailsHandle = nullptr;
					EOS_LobbySearch_CopySearchResultByIndex(searchHandle, &getOpts, &detailsHandle);

					if (numSessions == 0)
					{
						NetworkLog("[NGMP] Failed to find lobby to join!\n");

						if (NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby != nullptr)
						{
							NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby(false);
						}
					}
					else
					{
						EOS_Lobby_JoinLobbyOptions joinOpts;
						joinOpts.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
						joinOpts.LobbyDetailsHandle = detailsHandle;
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
									NetworkLog("[NGMP] Joined lobby!");

									NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_strCurrentLobbyID = Data->LobbyId;
									NetworkLog("[NGMP] Joined lobby!\n");

									// TODO_NGMP: Impl
									NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->OnJoinedOrCreatedLobby();
									// Set our properties
									NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->ResetCachedRoomData();
									NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->ApplyLocalUserPropertiesToCurrentNetworkRoom();


									if (NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby != nullptr)
									{
										NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby(true);
									}
								}
								else
								{
									NetworkLog("[NGMP] Failed to join lobby!");
									if (NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby != nullptr)
									{
										NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby(false);
									}
								}
							});
					}
				}
				else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
				{
					NetworkLog("[NGMP] Failed to find lobby to join!\n");

					if (NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby != nullptr)
					{
						NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby(false);
					}
				}
				else
				{
					NetworkLog("[NGMP] Failed to search for Lobbies!");

					if (NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby != nullptr)
					{
						NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_callbackJoinedLobby(false);
					}
				}
			});
	}
}

void NGMP_OnlineServices_LobbyInterface::CreateLobby(UnicodeString strLobbyName, UnicodeString strInitialMapName, AsciiString strInitialMapPath, int initialMaxSize)
{
	m_PendingCreation_LobbyName = strLobbyName;
	m_PendingCreation_InitialMapDisplayName = strInitialMapName;
	m_PendingCreation_InitialMapPath = strInitialMapPath;

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

					AsciiString strName = AsciiString();
					strName.translate(NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingCreation_LobbyName);
					AttributeData.Value.AsUtf8 = strName.str();

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

				// MAP NAME
				{
					EOS_LobbyModification_AddAttributeOptions AddAttributeModOptions;
					EOS_Lobby_AttributeData AttributeData;
					AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
					AttributeData.Key = "MAP_NAME";
					AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;
;
					AsciiString strMapName = AsciiString();
					strMapName.translate(NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingCreation_InitialMapDisplayName);
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

				// MAP PATH
				{
					EOS_LobbyModification_AddAttributeOptions AddAttributeModOptions;
					EOS_Lobby_AttributeData AttributeData;
					AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
					AttributeData.Key = "MAP_PATH";
					AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;
					
					AsciiString strMapPath = NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->m_PendingCreation_InitialMapPath;
					AttributeData.Value.AsUtf8 = strMapPath.str();

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

				NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->OnJoinedOrCreatedLobby();
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

void NGMP_OnlineServices_LobbyInterface::OnJoinedOrCreatedLobby()
{
	// TODO_NGMP: Cleanup game + dont store 2 ptrs
	m_pGameInst = new NGMPGame();
	TheNGMPGame = m_pGameInst;

	// inform game instance too
	if (TheNGMPGame != nullptr)
	{
		TheNGMPGame->UpdateSlotsFromCurrentLobby();
	}
}
