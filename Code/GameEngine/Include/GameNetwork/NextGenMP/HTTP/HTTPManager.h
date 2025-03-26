#pragma once

#include "HTTPRequest.h"
#include "../../../../../Libraries/libcurl/include/curl/multi.h"
#include <vector>
#include <mutex>
#include <thread>

enum class EHTTPVerb
{
	GET,
	POST
};

enum class EIPProtocolVersion
{
	DONT_CARE = 0,
	FORCE_IPV4 = 4,
	FORCE_IPV6 = 6
};

class HTTPManager
{
public:
	HTTPManager() noexcept;
	~HTTPManager();

	void MainThreadTick();

	void SendGETRequest(const char* szURI, EIPProtocolVersion protover, std::map<std::string, std::string>& inHeaders, std::function<void(bool bSuccess, int statusCode, std::string strBody)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback = nullptr);
	void SendPOSTRequest(const char* szURI, EIPProtocolVersion protover, std::map<std::string, std::string>& inHeaders, const char* szPostData, std::function<void(bool bSuccess, int statusCode, std::string strBody)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback = nullptr);

	void Shutdown();

	void BackgroundThreadRun();

	char* PlatformEscapeString(const char* szString, int len);


	CURLM* GetMultiHandle() { return m_pCurl; }
private:
	HTTPRequest* PlatformCreateRequest(EHTTPVerb htpVerb, EIPProtocolVersion protover, const char* szURI, std::map<std::string, std::string>& inHeaders, std::function<void(bool bSuccess, int statusCode, std::string strBody)> completionCallback,
		std::function<void(size_t bytesReceived)> progressCallback = nullptr) noexcept;

	void PlatformThreadedTick_PreLock();
	void PlatformThreadedTick_Locked();

private:
	CURLM* m_pCurl = nullptr;

	std::thread* m_backgroundThread = nullptr;

	std::recursive_mutex m_mutex;
	std::vector<HTTPRequest*> m_vecRequestsPendingstart = std::vector<HTTPRequest*>();
	std::vector<HTTPRequest*> m_vecRequestsInFlight = std::vector<HTTPRequest*>();
};


