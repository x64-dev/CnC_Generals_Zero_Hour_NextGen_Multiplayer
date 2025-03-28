#pragma once

class NGMP_OnlineServices_AuthInterface
{
public:

	AsciiString GetDisplayName()
	{
		return AsciiString(m_strDisplayName.c_str());
	}

	EOS_ProductUserId GetEOSUser() const { return nullptr; }

	void BeginLogin();

	void Tick();

	void OnLoginComplete(bool bSuccess);

	void RegisterForLoginCallback(std::function<void(bool)> callback)
	{
		m_vecLogin_PendingCallbacks.push_back(callback);
	}

	std::string& GetAuthToken() { return m_strToken; }

private:
	void LoginAsSecondaryDevAccount();

	void SaveCredentials(const char* szToken);
	bool DoCredentialsExist();
	std::string GetCredentials();

private:
	bool m_bWaitingLogin = false;
	std::string m_strCode;
	std::int64_t m_lastCheckCode = -1;

	std::string m_strToken = std::string();
	int64_t m_userID = -1;
	std::string m_strDisplayName = "NO_USER";

	std::vector<std::function<void(bool)>> m_vecLogin_PendingCallbacks = std::vector<std::function<void(bool)>>();
};