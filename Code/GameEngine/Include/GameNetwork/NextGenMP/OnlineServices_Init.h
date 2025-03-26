#pragma once

#include "NGMP_include.h"
#include "PortMapper.h"

class HTTPManager;

class NGMP_OnlineServices_AuthInterface;
class NGMP_OnlineServices_LobbyInterface;
class NGMP_OnlineServices_RoomsInterface;

#pragma comment(lib, "EOSSDK-Win64-Shipping.lib")
#pragma comment(lib, "libcurl.lib")

class NetworkRoom
{
public:
	NetworkRoom(int roomID, const wchar_t* strRoomName, const char* szRoomInternalName)
	{
		m_RoomID = roomID;
		m_strRoomDisplayName = UnicodeString(strRoomName);
		m_strRoomInternalName = AsciiString(szRoomInternalName);
	}

	~NetworkRoom()
	{

	}

	int GetRoomID() const { return m_RoomID; }
	UnicodeString GetRoomDisplayName() const { return m_strRoomDisplayName; }
	AsciiString GetRoomInternalName() const { return m_strRoomInternalName; }

private:
	int m_RoomID;
	UnicodeString m_strRoomDisplayName;
	AsciiString m_strRoomInternalName;
};

enum NGMP_ENATType : uint8_t
{
	NAT_TYPE_UNDETERMINED,
	NAT_TYPE_OPEN,
	NAT_TYPE_MODERATE,
	NAT_TYPE_STRICT
};

class NGMP_OnlineServicesManager
{
private:
	static NGMP_OnlineServicesManager* m_pOnlineServicesManager;

public:

	NGMP_OnlineServicesManager();
	
	static NGMP_OnlineServicesManager* GetInstance()
	{
		return m_pOnlineServicesManager;
	}


	~NGMP_OnlineServicesManager()
	{
		if (m_pAuthInterface != nullptr)
		{
			delete m_pAuthInterface;
			m_pAuthInterface = nullptr;
		}

		if (m_pLobbyInterface != nullptr)
		{
			delete m_pLobbyInterface;
			m_pLobbyInterface = nullptr;
		}

		if (m_pRoomInterface != nullptr)
		{
			delete m_pRoomInterface;
			m_pRoomInterface = nullptr;
		}
	}

	HTTPManager* GetHTTPManager() const { return m_pHTTPManager; }

	NGMP_OnlineServices_AuthInterface* GetAuthInterface() const { return m_pAuthInterface; }
	NGMP_OnlineServices_LobbyInterface* GetLobbyInterface() const { return m_pLobbyInterface; }
	NGMP_OnlineServices_RoomsInterface* GetRoomsInterface() const { return m_pRoomInterface; }

	void Init();

	void Tick();

	EOS_HPlatform GetEOSPlatformHandle() {
		return m_EOSPlatformHandle;
	}

	PortMapper& GetPortMapper() { return m_PortMapper; }

	std::vector<NetworkRoom> GetGroupRooms()
	{
		return m_vecRooms;
	}

	std::function<void(NGMP_ENATType previousNATType, NGMP_ENATType newNATType)> m_cbNATTypeChanged = nullptr;
	void RegisterForNATTypeChanges(std::function<void(NGMP_ENATType previousNATType, NGMP_ENATType newNATType)> cbNATTypeChanged) { m_cbNATTypeChanged = cbNATTypeChanged; }

	void CacheNATType(NGMP_ENATType natType)
	{
		NGMP_ENATType oldNATType = m_NATType;
		
		m_NATType = natType;

		if (m_cbNATTypeChanged != nullptr)
		{
			m_cbNATTypeChanged(oldNATType, m_NATType);
		}
	}
	NGMP_ENATType GetNATType() const { return m_NATType; }
	AsciiString GetNATTypeString() const
	{
		switch (m_NATType)
		{
		default:
		case NGMP_ENATType::NAT_TYPE_UNDETERMINED:
			return AsciiString("Undetermined");

		case NGMP_ENATType::NAT_TYPE_OPEN:
			return AsciiString("Open");

		case NGMP_ENATType::NAT_TYPE_MODERATE:
			return AsciiString("Moderate");

		case NGMP_ENATType::NAT_TYPE_STRICT:
			return AsciiString("Strict");
		}

		return AsciiString("Undetermined");
	}

private:
	EOS_HPlatform m_EOSPlatformHandle = nullptr;

	// TODO_NGMP: Get this from title storage or session query instead
	std::vector<NetworkRoom> m_vecRooms =
	{
		NetworkRoom(0, L"Room 1", "NETWORK_ROOM_0"),
		NetworkRoom(1, L"Room 2", "NETWORK_ROOM_1")
	};

	NGMP_ENATType m_NATType = NGMP_ENATType::NAT_TYPE_UNDETERMINED;


	NGMP_OnlineServices_AuthInterface* m_pAuthInterface = nullptr;
	NGMP_OnlineServices_LobbyInterface* m_pLobbyInterface = nullptr;
	NGMP_OnlineServices_RoomsInterface* m_pRoomInterface = nullptr;
	PortMapper m_PortMapper;

	HTTPManager* m_pHTTPManager = nullptr;
};