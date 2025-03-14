#pragma once

#include "NGMP_include.h"


class NGMP_OnlineServices_AuthInterface;
class NGMP_OnlineServices_LobbyInterface;
class NGMP_OnlineServices_RoomsInterface;

#pragma comment(lib, "EOSSDK-Win64-Shipping.lib")
#pragma comment(lib, "steam_api64.lib")
#pragma comment(lib, "sdkencryptedappticket64.lib")

class NetworkRoom
{
public:
	NetworkRoom(int roomID, wchar_t* strRoomName)
	{
		m_RoomID = roomID;
		m_strRoomName = UnicodeString(strRoomName);
	}

	~NetworkRoom()
	{

	}

	int GetRoomID() const { return m_RoomID; }
	UnicodeString GetRoomName() const { return m_strRoomName; }

private:
	int m_RoomID;
	UnicodeString m_strRoomName;
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

	NGMP_OnlineServices_AuthInterface* GetAuthInterface() const { return m_pAuthInterface; }
	NGMP_OnlineServices_LobbyInterface* GetLobbyInterface() const { return m_pLobbyInterface; }
	NGMP_OnlineServices_RoomsInterface* GetRoomsInterface() const { return m_pRoomInterface; }
	
	void SendNetworkRoomChat(UnicodeString msg);

	void Init();

	void Tick();

	EOS_HPlatform GetEOSPlatformHandle() {
		return m_EOSPlatformHandle;
	}

	

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
		m_cbNATTypeChanged(oldNATType, m_NATType);
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
		NetworkRoom(0, L"First Ever Room!"),
		NetworkRoom(1, L"Another room!")
	};

	NGMP_ENATType m_NATType = NGMP_ENATType::NAT_TYPE_UNDETERMINED;


	NGMP_OnlineServices_AuthInterface* m_pAuthInterface = nullptr;
	NGMP_OnlineServices_LobbyInterface* m_pLobbyInterface = nullptr;
	NGMP_OnlineServices_RoomsInterface* m_pRoomInterface = nullptr;
};