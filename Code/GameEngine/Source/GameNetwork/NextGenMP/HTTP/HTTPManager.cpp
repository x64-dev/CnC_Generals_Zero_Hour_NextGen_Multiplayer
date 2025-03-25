#include "GameNetwork/NextGenMP/HTTP/HTTPManager.h"

HTTPManager::HTTPManager() noexcept
{
	m_pCurl = curl_multi_init();

	// HTTP background thread
	m_backgroundThread = new std::thread(&HTTPManager::BackgroundThreadRun, this);

	if (IsDebuggerPresent())
	{
// 		DWORD ThreadId = ::GetThreadId(static_cast<HANDLE>(m_backgroundThread->native_handle()));
// 		THREADNAME_INFO info;
// 		info.dwType = 0x1000;
// 		info.szName = "HTTP Background Thread";
// 		info.dwThreadID = ThreadId;
// 		info.dwFlags = 0;
// 		RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
}

void HTTPManager::SendGETRequest(const char* szURI, std::map<std::string, std::string>& inHeaders, std::function<void(bool bSuccess, int statusCode, std::string strBody)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback)
{
	HTTPRequest* pRequest = PlatformCreateRequest(EHTTPVerb::GET, szURI, inHeaders, completionCallback, progressCallback);

	m_mutex.lock();
	m_vecRequestsPendingstart.push_back(pRequest);
	m_mutex.unlock();
}

void HTTPManager::SendPOSTRequest(const char* szURI, std::map<std::string, std::string>& inHeaders, const char* szPostData, std::function<void(bool bSuccess, int statusCode, std::string strBody)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback)
{
	HTTPRequest* pRequest = PlatformCreateRequest(EHTTPVerb::POST, szURI, inHeaders, completionCallback, progressCallback);
	pRequest->SetPostData(szPostData);

	m_mutex.lock();
	m_vecRequestsPendingstart.push_back(pRequest);
	m_mutex.unlock();
}

void HTTPManager::PlatformThreadedTick_PreLock()
{
	// We do this pre-lock becaues because it doesn't access anything shared (m_vecRequestsInFlight etc), but it does block/consume time, so locking means the game thread wont run logic for a while, dropping fps
	int numReqs = 0;
	curl_multi_perform(m_pCurl, &numReqs);
	curl_multi_poll(m_pCurl, NULL, 0, 1, NULL);
}

void HTTPManager::PlatformThreadedTick_Locked()
{
	// are we done?
	int msgq = 0;
	CURLMsg* m = curl_multi_info_read(m_pCurl, &msgq);
	if (m && (m->msg == CURLMSG_DONE))
	{
		CURL* e = m->easy_handle;

		// find the associated request
		for (HTTPRequest* pRequest : m_vecRequestsInFlight)
		{
			HTTPRequest* pPlatformRequest = static_cast<HTTPRequest*>(pRequest);
			if (pPlatformRequest->EasyHandleMatches(e))
			{
				pPlatformRequest->Threaded_SetComplete();
			}
		}
	}
}

void HTTPManager::Shutdown()
{

}

void HTTPManager::BackgroundThreadRun()
{
	// TODO_HTTP: While should be until core is existing
	while (true)
	{
		PlatformThreadedTick_PreLock();

		m_mutex.lock();

		PlatformThreadedTick_Locked();

		for (HTTPRequest* pRequest : m_vecRequestsPendingstart)
		{
			if (!pRequest->HasStarted())
			{
				pRequest->StartRequest();
			}

			// add to the proper queue
			m_vecRequestsInFlight.push_back(pRequest);
		}
		m_vecRequestsPendingstart.clear();

		m_mutex.unlock();

		Sleep(1);
	}
}


char* HTTPManager::PlatformEscapeString(const char* szString, int len)
{
	CURL* pCURL = curl_easy_init();
	char* pEsc = curl_easy_escape(pCURL, szString, len);

	// TODO_NGMP: Delete pcurl or keep a persistent one around?
	//delete pCURL;
	return pEsc;
}

HTTPRequest* HTTPManager::PlatformCreateRequest(EHTTPVerb httpVerb, const char* szURI, std::map<std::string, std::string>& inHeaders, std::function<void(bool bSuccess, int statusCode, std::string strBody)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback /*= nullptr*/) noexcept
{
	HTTPRequest* pNewRequest = new HTTPRequest(httpVerb, szURI, inHeaders, completionCallback, progressCallback);
	return pNewRequest;
}

HTTPManager::~HTTPManager()
{
	curl_multi_cleanup(m_pCurl);
}

void HTTPManager::MainThreadTick()
{
	m_mutex.lock();

	std::vector<HTTPRequest*> vecItemsToRemove = std::vector<HTTPRequest*>();
	for (HTTPRequest* pRequest : m_vecRequestsInFlight)
	{
		// do we need a progress update? We do this here so it comes from the main thread so that UI etc can consume it
		if (pRequest->NeedsProgressUpdate())
		{
			pRequest->InvokeProgressUpdateCallback();
		}

		// are we done?
		if (pRequest->IsComplete())
		{
			vecItemsToRemove.push_back(pRequest);

			// trigger callback
			pRequest->InvokeCallbackIfComplete();
		}
	}

	for (HTTPRequest* pRequestToDestroy : vecItemsToRemove)
	{
		m_vecRequestsInFlight.erase(std::remove(m_vecRequestsInFlight.begin(), m_vecRequestsInFlight.end(), pRequestToDestroy));
		delete pRequestToDestroy;
	}

	m_mutex.unlock();
}
