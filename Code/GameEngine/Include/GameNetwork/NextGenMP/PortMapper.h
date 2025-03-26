#pragma once

#define NATPMP_STATICLIB 1
#pragma comment(lib, "iphlpapi.lib")

#include "common/gsPlatformSocket.h"
#include "GameNetwork/NextGenMP/Vendor/libnatpmp/natpmp.h"
#include "../miniupnpc/include/miniupnpc.h"
#include "../miniupnpc/include/miniupnpctypes.h"
#include "../miniupnpc/include/upnperrors.h"
#include "../miniupnpc/include/upnpcommands.h"
#pragma comment(lib, "miniupnpc.lib")
#include <functional>

enum ECapabilityState : uint8_t
{
	UNDETERMINED,
	UNSUPPORTED,
	SUPPORTED
};

class PortMapper
{
public:
	~PortMapper()
	{
		CleanupPorts();
	}

	void DetermineLocalNetworkCapabilities(std::function<void(void)> callbackDeterminedCaps);

	void TryForwardPreferredPorts();
	void CleanupPorts();

	ECapabilityState HasUPnP()
	{
		return m_capUPnP;
	}

	ECapabilityState HasNATPMP()
	{
		return m_capNATPMP;
	}

	ECapabilityState HasIPv6()
	{
		return m_capIPv4;
	}

	ECapabilityState HasIPv4()
	{
		return m_capIPv6;
	}

	bool HasPortOpen() const { return m_bHasPortOpenedViaUPNP || m_bHasPortOpenedViaNATPMP; }
	bool HasPortOpenUPnP() const { return m_bHasPortOpenedViaUPNP; }
	bool HasPortOpenNATPMP() const { m_bHasPortOpenedViaNATPMP; }
	int GetOpenPort_External() const { return m_PreferredPortExternal; }
	int GetOpenPort_Internal() const { return m_PreferredPortInternal; }

private:
	bool ForwardPreferredPort_UPnP();
	bool ForwardPreferredPort_NATPMP();

	void UPnP_RemoveAllMappingsToThisMachine();
	void RemovePortMapping_UPnP();
	void RemovePortMapping_NATPMP();

private:
	std::function<void(void)> m_callbackDeterminedCaps = nullptr;
	ECapabilityState m_capUPnP = ECapabilityState::UNDETERMINED;
	ECapabilityState m_capNATPMP = ECapabilityState::UNDETERMINED;
	ECapabilityState m_capIPv4 = ECapabilityState::UNDETERMINED;
	ECapabilityState m_capIPv6 = ECapabilityState::UNDETERMINED;

	bool m_bHasPortOpenedViaUPNP = false;
	bool m_bHasPortOpenedViaNATPMP = false;

	uint16_t m_PreferredPortInternal = 0;
	uint16_t m_PreferredPortExternal = 0;
};