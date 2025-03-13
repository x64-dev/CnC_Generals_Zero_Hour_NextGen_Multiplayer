#include "GameNetwork/NextGenMP/NGMP_OnlineServices_Init.h"
#include "common/gsPlatformUtil.h"
#include <fstream>
#include "GameNetwork/NextGenMP/NextGenMP_defines.h"

NGMP_OnlineServicesManager* NGMP_OnlineServicesManager::m_pOnlineServicesManager = nullptr;


NGMP_OnlineServicesManager::NGMP_OnlineServicesManager()
{
	OutputDebugString("NGMP: Init");
	m_pOnlineServicesManager = this;
}

void NGMP_OnlineServicesManager::SendNetworkRoomChat(UnicodeString msg)
{

}

void NGMP_OnlineServicesManager::Init()
{
	bool bSteamInit = SteamAPI_Init();

	if (!bSteamInit)
	{
		OutputDebugString("NGMP: Steam initialization failed");
	}

	// Init EOS SDK
	EOS_InitializeOptions SDKOptions = {};
	SDKOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
	SDKOptions.AllocateMemoryFunction = nullptr;
	SDKOptions.ReallocateMemoryFunction = nullptr;
	SDKOptions.ReleaseMemoryFunction = nullptr;

	char szBuffer[MAX_PATH] = { 0 };
	strcpy(szBuffer, "Generals");
	SDKOptions.ProductName = szBuffer;
	SDKOptions.ProductVersion = "1.0";
	SDKOptions.Reserved = nullptr;
	SDKOptions.SystemInitializeOptions = nullptr;
	SDKOptions.OverrideThreadAffinity = nullptr;

	EOS_EResult InitResult = EOS_Initialize(&SDKOptions);

	// TODO_LAUNCH: Have a define to turn off all debug logging, even EOS below, should be no logging on retail builds
	if (InitResult == EOS_EResult::EOS_Success)
	{
		// LOGGING
		EOS_EResult SetLogCallbackResult = EOS_Logging_SetCallback([](const EOS_LogMessage* Message)
			{
				OutputDebugString(Message->Message);
				OutputDebugString("\n");
			});
		if (SetLogCallbackResult != EOS_EResult::EOS_Success)
		{
			OutputDebugString("NGMP: Failed to set EOS log callback");
		}
		else
		{

#if _DEBUG
			EOS_Logging_SetLogLevel(EOS_ELogCategory::EOS_LC_ALL_CATEGORIES, EOS_ELogLevel::EOS_LOG_Info);
#else
			EOS_Logging_SetLogLevel(EOS_ELogCategory::EOS_LC_ALL_CATEGORIES, EOS_ELogLevel::EOS_LOG_Info);
#endif
		}

		char CurDir[MAX_PATH + 1] = {};
		::GetCurrentDirectoryA(MAX_PATH + 1u, CurDir);


		// cache dir
		char szTempDir[MAX_PATH + 1] = { 0 };
		sprintf(szTempDir, "%s\\eos_cache", CurDir);

		// PLATFORM OPTIONS
		EOS_Platform_Options PlatformOptions = {};
		PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
		PlatformOptions.bIsServer = EOS_FALSE;
		PlatformOptions.OverrideCountryCode = nullptr;
		PlatformOptions.OverrideLocaleCode = nullptr;
		PlatformOptions.Flags = EOS_PF_WINDOWS_ENABLE_OVERLAY_D3D9 | EOS_PF_WINDOWS_ENABLE_OVERLAY_D3D10;
		PlatformOptions.CacheDirectory = szTempDir;

		PlatformOptions.ProductId = NGMP_EOS_PRODUCT_ID;
		PlatformOptions.SandboxId = NGMP_EOS_SANDBOX_ID;
		PlatformOptions.EncryptionKey = NGMP_EOS_ENCRYPTION_KEY;
		PlatformOptions.DeploymentId = NGMP_EOS_DEPLOYMENT_ID;
		PlatformOptions.ClientCredentials.ClientId = NGMP_EOS_CLIENT_ID;
		PlatformOptions.ClientCredentials.ClientSecret = NGMP_EOS_CLIENT_SECRET;

		double timeout = 5000.f;
		PlatformOptions.TaskNetworkTimeoutSeconds = &timeout;

		EOS_Platform_RTCOptions RtcOptions = { 0 };
		RtcOptions.ApiVersion = EOS_PLATFORM_RTCOPTIONS_API_LATEST;

		// Get absolute path for xaudio2_9redist.dll file
		char szXAudioDir[MAX_PATH + 1] = { 0 };
		sprintf(szXAudioDir, "%s\\xaudio2_9redist.dll", CurDir);

		// does the DLL exist on disk?
		std::fstream fileStream;
		fileStream.open(szXAudioDir, std::fstream::in | std::fstream::binary);
		if (!fileStream.good())
		{
			OutputDebugString("NGMP: FATAL ERROR: Failed to locate XAudio DLL");
			exit(1);
		}
		else
		{
			OutputDebugString("NGMP: XAudio DLL located successfully");
		}


		EOS_Windows_RTCOptions WindowsRtcOptions = { 0 };
		WindowsRtcOptions.ApiVersion = EOS_WINDOWS_RTCOPTIONS_API_LATEST;
		WindowsRtcOptions.XAudio29DllPath = szXAudioDir;
		RtcOptions.PlatformSpecificOptions = &WindowsRtcOptions;
		
		PlatformOptions.RTCOptions = &RtcOptions;

#if ALLOW_RESERVED_PLATFORM_OPTIONS
		SetReservedPlatformOptions(PlatformOptions);
#else
		PlatformOptions.Reserved = NULL;
#endif // ALLOW_RESERVED_PLATFORM_OPTIONS

		// platform integration settings
		// Create the generic container.
		const EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainerOptions CreateOptions =
		{
			EOS_INTEGRATEDPLATFORM_CREATEINTEGRATEDPLATFORMOPTIONSCONTAINER_API_LATEST
		};

		const EOS_EResult Result = EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainer(&CreateOptions, &PlatformOptions.IntegratedPlatformOptionsContainerHandle);
		// TODO_NGMP: Handle error
		if (Result != EOS_EResult::EOS_Success)
		{
			OutputDebugString("NGMP: EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainer returned an error\n");
		}

		// Configure platform-specific options.
		const EOS_IntegratedPlatform_Steam_Options PlatformSpecificOptions =
		{
				EOS_INTEGRATEDPLATFORM_STEAM_OPTIONS_API_LATEST,
				nullptr,
				1,
				59
		};

		// Add the configuration to the SDK initialization options.
		const EOS_IntegratedPlatform_Options Options =
		{
			EOS_INTEGRATEDPLATFORM_OPTIONS_API_LATEST,
			EOS_IPT_Steam,
			EOS_EIntegratedPlatformManagementFlags::EOS_IPMF_LibraryManagedByApplication | EOS_EIntegratedPlatformManagementFlags::EOS_IPMF_DisableSDKManagedSessions | EOS_EIntegratedPlatformManagementFlags::EOS_IPMF_PreferIntegratedIdentity,
			&PlatformSpecificOptions
		};

		const EOS_IntegratedPlatformOptionsContainer_AddOptions AddOptions =
		{
			EOS_INTEGRATEDPLATFORMOPTIONSCONTAINER_ADD_API_LATEST,
			&Options
		};
		//EOS_IntegratedPlatformOptionsContainer_Add(PlatformOptions.IntegratedPlatformOptionsContainerHandle, &AddOptions);

		// end platform integration settings

		// TODO_NGMP: We dont EOS_Platform_Release or shutdown the below
		m_EOSPlatformHandle = EOS_Platform_Create(&PlatformOptions);
		// TODO_NGMP: Don't process any input if steam or eos overlays are showing
	}
}

void NGMP_OnlineServicesManager::Auth()
{
	SteamNetworkingIdentity steamNetIdentity;
	steamNetIdentity.SetSteamID(SteamUser()->GetSteamID());

	bool bSteamOnline = SteamUser()->BLoggedOn();

	HAuthTicket steamAuthTicket = SteamUser()->GetAuthTicketForWebApi("epiconlineservices");

	if (steamAuthTicket == k_HAuthTicketInvalid || !bSteamOnline)
	{
		OutputDebugString("SteamUser()->GetAuthSessionTicket returned invalid auth ticket.");
		OnAuthSessionTicketResponse(nullptr);

		// TODO_NGMP: Error out here, we're trying to play online while offline (or steam issues etc)
	}
}

void NGMP_OnlineServicesManager::Tick()
{
	SteamAPI_RunCallbacks();

	if (m_EOSPlatformHandle != nullptr)
	{
		EOS_Platform_Tick(m_EOSPlatformHandle);
	}
}

void NGMP_OnlineServicesManager::LoginToEpic()
{
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(m_EOSPlatformHandle);

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

	EOS_Connect_Login(ConnectHandle, &loginOpts, this, [](const EOS_Connect_LoginCallbackInfo* Data)
		{
			NGMP_OnlineServicesManager* pOnlineServicesMgr = (NGMP_OnlineServicesManager*)Data->ClientData;
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				OutputDebugString("EOS Login worked");

				pOnlineServicesMgr->OnEpicLoginComplete(Data->LocalUserId);
			}
			else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
			{
				EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(pOnlineServicesMgr->GetEOSPlatformHandle());
				assert(ConnectHandle != NULL);

				EOS_Connect_CreateUserOptions Options;
				Options.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;

				if (Data->ContinuanceToken != NULL)
				{
					Options.ContinuanceToken = Data->ContinuanceToken;
				}

				// NOTE: We're not deleting the received context because we're passing it down to another SDK call
				EOS_Connect_CreateUser(ConnectHandle, &Options, pOnlineServicesMgr,
					[](const EOS_Connect_CreateUserCallbackInfo* Data)
					{
						if (Data->ResultCode == EOS_EResult::EOS_Success)
						{
							OutputDebugString("Account Link Complete");

							NGMP_OnlineServicesManager* pOnlineServicesMgr = (NGMP_OnlineServicesManager*)Data->ClientData;
							pOnlineServicesMgr->OnEpicLoginComplete(Data->LocalUserId);
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
				OutputDebugString("EOS Login failed\n");
			}

		});
}

void NGMP_OnlineServicesManager::OnEpicLoginComplete(EOS_ProductUserId userID)
{
	m_EOSUserID = userID;

	char szUserID[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
	int len = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
	EOS_EResult r = EOS_ProductUserId_ToString(userID, szUserID, &len);
	if (r != EOS_EResult::EOS_Success)
	{
		// TODO_NGMP: Error
		OutputDebugString("EOS error!\n");
	}

	OutputDebugString("EOS Logged in!\n");
	m_strDisplayName = SteamFriends()->GetPersonaName();

	for (auto cb : m_vecLogin_PendingCallbacks)
	{
		// TODO_NGMP: Support failure
		cb(true);
	}
	m_vecLogin_PendingCallbacks.clear();
}

void NGMP_OnlineServicesManager::OnAuthSessionTicketResponse(GetTicketForWebApiResponse_t* pAuthSessionTicketResponse)
{
#if defined(__llvm__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch" // Steam does weird things with enum offsets, so we aren't actually missing 100+ valid cases
#endif

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
			// info is already in the buffer at this point



			// copy info to buffer
			m_vecSteamAuthSessionTicket.resize(pAuthSessionTicketResponse->m_cubTicket);
			memcpy(m_vecSteamAuthSessionTicket.data(), pAuthSessionTicketResponse->m_rgubTicket, pAuthSessionTicketResponse->m_cubTicket);

			LoginToEpic();
		}
		break;
		case k_EResultNoConnection:
			OutputDebugString("Calling RequestEncryptedAppTicket while not connected to steam results in this error.\n");
			break;
		case k_EResultDuplicateRequest:
			// TODO_NGMP: Dont fail here, just retry again in 10 sec?
			OutputDebugString("Calling RequestEncryptedAppTicket while there is already a pending request results in this error.\n");
			break;
		case k_EResultLimitExceeded:
			OutputDebugString("Calling RequestEncryptedAppTicket more than once per minute returns this error.\n");
			break;
		}
	}

	// trigger callback
	//Core::Get()->GetOnlineServicesManager()->GetAuthManager()->TriggerRequestPlatformTokenCallback(!Core::Get()->GetOnlineServicesManager()->IsOfflineMode());

#if defined(__llvm__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
}