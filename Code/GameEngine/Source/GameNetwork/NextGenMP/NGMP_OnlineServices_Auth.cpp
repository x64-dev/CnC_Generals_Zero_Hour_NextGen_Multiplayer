#include "GameNetwork/NextGenMP/NGMP_interfaces.h"

void NGMP_OnlineServices_AuthInterface::Auth()
{
	SteamNetworkingIdentity steamNetIdentity;
	steamNetIdentity.SetSteamID(SteamUser()->GetSteamID());

	bool bSteamOnline = SteamUser()->BLoggedOn();

	HAuthTicket steamAuthTicket = SteamUser()->GetAuthTicketForWebApi("epiconlineservices");

	if (steamAuthTicket == k_HAuthTicketInvalid || !bSteamOnline)
	{
		NetworkLog("[NGMP] SteamUser()->GetAuthSessionTicket returned invalid auth ticket.");
		OnAuthSessionTicketResponse(nullptr);

		// TODO_NGMP: Error out here, we're trying to play online while offline (or steam issues etc)
	}
}

void NGMP_OnlineServices_AuthInterface::OnAuthSessionTicketResponse(GetTicketForWebApiResponse_t* pAuthSessionTicketResponse)
{
	if (pAuthSessionTicketResponse == nullptr)
	{
		// TODO: ERROR
	}
	else
	{
		switch (pAuthSessionTicketResponse->m_eResult)
		{
		case k_EResultOK:
		{
			// copy info to buffer
			m_vecSteamAuthSessionTicket.resize(pAuthSessionTicketResponse->m_cubTicket);
			memcpy(m_vecSteamAuthSessionTicket.data(), pAuthSessionTicketResponse->m_rgubTicket, pAuthSessionTicketResponse->m_cubTicket);

			LoginToEpic();
		}
		break;
		case k_EResultNoConnection:
			NetworkLog("[NGMP] Calling RequestEncryptedAppTicket while not connected to steam results in this error.\n");
			break;
		case k_EResultDuplicateRequest:
			// TODO_NGMP: Dont fail here, just retry again in 10 sec?
			NetworkLog("[NGMP] Calling RequestEncryptedAppTicket while there is already a pending request results in this error.\n");
			break;
		case k_EResultLimitExceeded:
			NetworkLog("[NGMP] Calling RequestEncryptedAppTicket more than once per minute returns this error.\n");
			break;
		}
	}
}

void NGMP_OnlineServices_AuthInterface::LoginToEpic()
{
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	// parse the steam token
	uint32 StringBufSize = (m_vecSteamAuthSessionTicket.size() * 2) + 1;
	char* SteamAppTicketString = new char[StringBufSize];
	uint32_t OutLen = StringBufSize;
	EOS_EResult ConvResult = EOS_ByteArray_ToString(m_vecSteamAuthSessionTicket.data(), m_vecSteamAuthSessionTicket.size(), SteamAppTicketString, &OutLen);

	if (ConvResult != EOS_EResult::EOS_Success)
	{
		// TODO_NGMP: What do we do if this fails?
	}

	EOS_Connect_Credentials* creds = new EOS_Connect_Credentials();
	creds->ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	creds->Token = SteamAppTicketString;
	creds->Type = EOS_EExternalCredentialType::EOS_ECT_STEAM_SESSION_TICKET;

	SteamAppTicketString = nullptr;
	delete[] SteamAppTicketString;
	// done parsing steam token

	EOS_Connect_UserLoginInfo* userlogininfo = new EOS_Connect_UserLoginInfo();
	userlogininfo->ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
	userlogininfo->DisplayName = nullptr;
	userlogininfo->NsaIdToken = nullptr;

	EOS_Connect_LoginOptions loginOpts;
	loginOpts.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	loginOpts.Credentials = creds;
	loginOpts.UserLoginInfo = userlogininfo;

	EOS_Connect_Login(ConnectHandle, &loginOpts, nullptr, [](const EOS_Connect_LoginCallbackInfo* Data)
		{
			// TODO_NGMP: Need ptr safety all over the place...
			NGMP_OnlineServicesManager* pOnlineServicesMgr = NGMP_OnlineServicesManager::GetInstance();
			NGMP_OnlineServices_AuthInterface* pAuthInterface = pOnlineServicesMgr->GetAuthInterface();
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				NetworkLog("[NGMP] EOS Login worked");

				pAuthInterface->OnEpicLoginComplete(Data->LocalUserId);
			}
			else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
			{
				EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(pOnlineServicesMgr->GetEOSPlatformHandle());

				EOS_Connect_CreateUserOptions Options;
				Options.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;

				if (Data->ContinuanceToken != NULL)
				{
					Options.ContinuanceToken = Data->ContinuanceToken;
				}

				// NOTE: We're not deleting the received context because we're passing it down to another SDK call
				EOS_Connect_CreateUser(ConnectHandle, &Options, pAuthInterface,
					[](const EOS_Connect_CreateUserCallbackInfo* Data)
					{
						if (Data->ResultCode == EOS_EResult::EOS_Success)
						{
							NetworkLog("[NGMP] Account Link Complete");

							NGMP_OnlineServices_AuthInterface* pAuthInterface = (NGMP_OnlineServices_AuthInterface*)Data->ClientData;
							pAuthInterface->OnEpicLoginComplete(Data->LocalUserId);
						}
						else
						{
							// TODO_NGMP: Error
						}
					}
				);
			}
			else
			{
				NetworkLog("[NGMP] EOS Login failed\n");
			}

		});
}

void NGMP_OnlineServices_AuthInterface::OnEpicLoginComplete(EOS_ProductUserId userID)
{
	m_EOSUserID = userID;

	char szUserID[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
	int len = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
	EOS_EResult r = EOS_ProductUserId_ToString(userID, szUserID, &len);
	if (r != EOS_EResult::EOS_Success)
	{
		// TODO_NGMP: Error
		NetworkLog("[NGMP] EOS error!\n");
	}

	NetworkLog("[NGMP] EOS Logged in!\n");
	m_strDisplayName = SteamFriends()->GetPersonaName();

	EOS_HP2P P2PHandle = EOS_Platform_GetP2PInterface(NGMP_OnlineServicesManager::GetInstance()->GetEOSPlatformHandle());

	// start caching NAT type immediately, its asynchronous and requires packet transmission to a remote server
	EOS_P2P_QueryNATTypeOptions queryNatOpts;
	queryNatOpts.ApiVersion = EOS_P2P_QUERYNATTYPE_API_LATEST;
	EOS_P2P_QueryNATType(P2PHandle, &queryNatOpts, nullptr, [](const EOS_P2P_OnQueryNATTypeCompleteInfo* Data)
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				// convert NAT type to our enum for abstraction
				NGMP_ENATType natType = NGMP_ENATType::NAT_TYPE_UNDETERMINED;
				switch (Data->NATType)
				{
				default:
				case EOS_ENATType::EOS_NAT_Unknown:
					natType = NGMP_ENATType::NAT_TYPE_UNDETERMINED;
					break;

				case EOS_ENATType::EOS_NAT_Open:
					natType = NGMP_ENATType::NAT_TYPE_OPEN;
					break;

				case EOS_ENATType::EOS_NAT_Moderate:
					natType = NGMP_ENATType::NAT_TYPE_MODERATE;
					break;

				case EOS_ENATType::EOS_NAT_Strict:
					natType = NGMP_ENATType::NAT_TYPE_STRICT;
					break;
				}
				NGMP_OnlineServicesManager::GetInstance()->CacheNATType(natType);
			}
			else
			{
				NetworkLog("[NGMP] NAT Type Query Failed\n");
			}
		});

	// always allow relays
	EOS_P2P_SetRelayControlOptions relayOpts;
	relayOpts.ApiVersion = EOS_P2P_SETRELAYCONTROL_API_LATEST;
	relayOpts.RelayControl = EOS_ERelayControl::EOS_RC_AllowRelays;
	EOS_P2P_SetRelayControl(P2PHandle, &relayOpts);

	for (auto cb : m_vecLogin_PendingCallbacks)
	{
		// TODO_NGMP: Support failure
		cb(true);
	}
	m_vecLogin_PendingCallbacks.clear();
}