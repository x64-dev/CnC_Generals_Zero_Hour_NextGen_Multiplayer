#pragma once

class NGMP_OnlineServices_AuthInterface
{
public:

	void Auth();

	AsciiString GetDisplayName()
	{
		return AsciiString(m_strDisplayName.c_str());
	}
	STEAM_CALLBACK(NGMP_OnlineServices_AuthInterface, OnAuthSessionTicketResponse, GetTicketForWebApiResponse_t);

	EOS_ProductUserId GetEOSUser() const { return m_EOSUserID; }

	void LoginToEpic();

	void OnEpicLoginComplete(EOS_ProductUserId userID);

	void RegisterForLoginCallback(std::function<void(bool)> callback)
	{
		m_vecLogin_PendingCallbacks.push_back(callback);
	}

private:
	EOS_ProductUserId m_EOSUserID = nullptr;

	std::vector<uint8> m_vecSteamAuthSessionTicket;
	std::string m_strDisplayName = "NO_USER";
	std::vector<std::function<void(bool)>> m_vecLogin_PendingCallbacks = std::vector<std::function<void(bool)>>();
};