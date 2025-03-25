#include "GameNetwork/NextGenMP/NGMP_interfaces.h"

#include "GameNetwork/NextGenMP/HTTP/HTTPManager.h"
#include "GameNetwork/NextGenMP/HTTP/HTTPRequest.h"
#include <shellapi.h>
#include <algorithm>
#include <chrono>
#include <random>
#include <windows.h>
#include <wincred.h>
#include "GameNetwork/NextGenMP/json.hpp"

enum class EAuthResponseResult : int
{
	CODE_INVALID = -1,
	WAITING_USER_ACTION = 0,
	SUCCEEDED = 1,
	FAILED = 2
};

struct AuthResponse
{
	EAuthResponseResult result;
	std::string token;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AuthResponse, result, token)
};

std::string generateRandomString() {
	std::string result;
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const size_t max_index = sizeof(charset) - 1;

	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 generator(seed);
	std::uniform_int_distribution<> distribution(0, max_index - 1);

	for (int i = 0; i < 5; ++i) {
		result += charset[distribution(generator)];
	}

	return result;
}

void NGMP_OnlineServices_AuthInterface::BeginLogin()
{
	if (DoCredentialsExist())
	{
		std::string strToken = GetCredentials();

		// login
		std::string strLoginURI = std::format("https://www.playgenerals.online/login/do_login.php?token={}", strToken.c_str());
		std::map<std::string, std::string> mapHeaders;
		NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendGETRequest(strLoginURI.c_str(), mapHeaders, [=](bool bSuccess, int statusCode, std::string strBody)
		{
			nlohmann::json jsonObject = nlohmann::json::parse(strBody);
			AuthResponse authResp = jsonObject.get<AuthResponse>();

			if (authResp.result == EAuthResponseResult::SUCCEEDED)
			{
				NetworkLog("LOGIN: Logged in");
				m_bWaitingLogin = false;

				SaveCredentials(authResp.token.c_str());

				// trigger callback
				for (auto cb : m_vecLogin_PendingCallbacks)
				{
					// TODO_NGMP: Support failure
					cb(true);
				}
				m_vecLogin_PendingCallbacks.clear();
			}
			else if (authResp.result == EAuthResponseResult::FAILED)
			{
				NetworkLog("LOGIN: Login failed, trying to re-auth");

				// do normal login flow, token is bad or expired etc
				m_bWaitingLogin = true;
				m_lastCheckCode = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::utc_clock::now().time_since_epoch()).count();
				m_strCode = generateRandomString();

				std::string strURI = std::format("https://www.playgenerals.online/login/?code={}", m_strCode.c_str());

				ShellExecuteA(NULL, "open", strURI.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}

		}, nullptr);
	}
	else
	{
		m_bWaitingLogin = true;
		m_lastCheckCode = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::utc_clock::now().time_since_epoch()).count();
		m_strCode = generateRandomString();

		std::string strURI = std::format("https://www.playgenerals.online/login/?code={}", m_strCode.c_str());

		ShellExecuteA(NULL, "open", strURI.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	
	return;

	// if it's not the first instance of Generals... use the dev account
	/*
	static HANDLE MPMutex = NULL;
	MPMutex = CreateMutex(NULL, FALSE, "685EAFF2-3216-4265-FFFF-251C5F4B82F3");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		NetworkLog("[NGMP] Secondary instance detected... using dev account for testing purposes");
		LoginAsSecondaryDevAccount();
	}
	*/
}

void NGMP_OnlineServices_AuthInterface::Tick()
{
	if (m_bWaitingLogin)
	{
		const int64_t timeBetweenChecks = 1000;
		int64_t currTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::utc_clock::now().time_since_epoch()).count();

		if (currTime - m_lastCheckCode >= timeBetweenChecks)
		{
			m_lastCheckCode = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::utc_clock::now().time_since_epoch()).count();

			// check again
			std::string strURI = std::format("https://www.playgenerals.online/login/check.php?code={}", m_strCode.c_str());
			std::map<std::string, std::string> mapHeaders;
			NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendGETRequest(strURI.c_str(), mapHeaders, [=](bool bSuccess, int statusCode, std::string strBody)
			{
					nlohmann::json jsonObject = nlohmann::json::parse(strBody);
					AuthResponse authResp = jsonObject.get<AuthResponse>();

					NetworkLog("PageBody: %s", strBody.c_str());
					if (authResp.result == EAuthResponseResult::CODE_INVALID)
					{
						NetworkLog("LOGIN: Code didnt exist, trying again soon");
					}
					else if (authResp.result == EAuthResponseResult::WAITING_USER_ACTION)
					{
						NetworkLog("LOGIN: Waiting for user action");
					}
					else if (authResp.result == EAuthResponseResult::SUCCEEDED)
					{
						NetworkLog("LOGIN: Logged in");
						m_bWaitingLogin = false;

						SaveCredentials(authResp.token.c_str());

						// trigger callback
						for (auto cb : m_vecLogin_PendingCallbacks)
						{
							// TODO_NGMP: Support failure
							cb(true);
						}
						m_vecLogin_PendingCallbacks.clear();
					}
					else if (authResp.result == EAuthResponseResult::FAILED)
					{
						NetworkLog("LOGIN: Login failed");
						m_bWaitingLogin = false;

						// trigger callback
						for (auto cb : m_vecLogin_PendingCallbacks)
						{
							// TODO_NGMP: Support failure
							cb(false);
						}
						m_vecLogin_PendingCallbacks.clear();
					}
				
			}, nullptr);
		}
	}
}

void NGMP_OnlineServices_AuthInterface::LoginAsSecondaryDevAccount()
{

}

void NGMP_OnlineServices_AuthInterface::SaveCredentials(const char* szToken)
{
	// store in credmgr
	DWORD blobsize = strlen(szToken);

	CREDENTIALA cred = { 0 };
	cred.Flags = 0;
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = (char*)"GeneralsOnline";
	cred.CredentialBlobSize = blobsize;
	cred.CredentialBlob = (LPBYTE)szToken;
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.UserName = (char*)"";

	if (!CredWrite(&cred, 0))
	{
		NetworkLog("ERROR STORING CREDENTIALS: %d\n", GetLastError());
	}
}

bool NGMP_OnlineServices_AuthInterface::DoCredentialsExist()
{
	std::string strToken = GetCredentials();
	return !strToken.empty();
}

std::string NGMP_OnlineServices_AuthInterface::GetCredentials()
{
	PCREDENTIAL credential;
	if (CredRead("GeneralsOnline", CRED_TYPE_GENERIC, 0, &credential))
	{
		std::string token = std::string((char*)credential->CredentialBlob, credential->CredentialBlobSize);
		CredFree(credential);

		if (token.length() == 32)
		{
			return token;
		}
	}

	return std::string();
}