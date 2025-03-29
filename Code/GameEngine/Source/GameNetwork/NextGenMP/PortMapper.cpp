#include "GameNetwork/NextGenMP/PortMapper.h"
#include "GameNetwork/NextGenMP/NGMP_include.h"
#include "GameNetwork/NextGenMP/NGMP_interfaces.h"

#include "GameNetwork/NextGenMP/HTTP/HTTPManager.h"
#include "GameNetwork/NextGenMP/HTTP/HTTPRequest.h"
#include "GameNetwork/NextGenMP/json.hpp"

#include <random>
#include <chrono>

struct IPCapsResult
{
	int ipversion;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(IPCapsResult, ipversion)
};

void PortMapper::DetermineLocalNetworkCapabilities(std::function<void(void)> callbackDeterminedCaps)
{
	// reset state
	m_capUPnP = ECapabilityState::UNDETERMINED;
	m_capNATPMP = ECapabilityState::UNDETERMINED;
	m_capIPv4 = ECapabilityState::UNDETERMINED;
	m_capIPv6 = ECapabilityState::UNDETERMINED;

	// store callback
	m_callbackDeterminedCaps = callbackDeterminedCaps;
	// TODO_NGMP: Do this on a background thread?
	
	// UPnP
	int errorCode = 0;
	struct UPNPDev* upnp_dev = upnpDiscover(2000, nullptr, nullptr, 0, 0, 2, &errorCode);

	char lan_address[64];
	char wan_address[64];
	struct UPNPUrls upnp_urls;
	struct IGDdatas upnp_data;
	int IGDStatus = UPNP_GetValidIGD(upnp_dev, &upnp_urls, &upnp_data, lan_address, sizeof(lan_address), wan_address, sizeof(wan_address));

	m_capUPnP = (IGDStatus == 1) ? ECapabilityState::SUPPORTED : ECapabilityState::UNSUPPORTED;

	// NAT-PMP
	int res;
	natpmp_t natpmp;
	natpmpresp_t response;
	initnatpmp(&natpmp, 0, 0);

	sendpublicaddressrequest(&natpmp);
	do
	{
		fd_set fds;
		struct timeval timeout;
		FD_ZERO(&fds);
		FD_SET(natpmp.s, &fds);
		getnatpmprequesttimeout(&natpmp, &timeout);
		select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
		res = readnatpmpresponseorretry(&natpmp, &response);
	} while (res == NATPMP_TRYAGAIN);

	closenatpmp(&natpmp);

	m_capNATPMP = (res == NATPMP_RESPTYPE_PUBLICADDRESS) ? ECapabilityState::SUPPORTED : ECapabilityState::UNSUPPORTED;;

	// open ports
	TryForwardPreferredPorts();

	// check IPv4
	std::map<std::string, std::string> mapHeaders;
	NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendGETRequest(std::format("https://playgenerals.online/cloud/env:dev:{}/DetermineIPCapabilities", NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetAuthToken()).c_str(), EIPProtocolVersion::FORCE_IPV4, mapHeaders, [=](bool bSuccess, int statusCode, std::string strBody)
		{
			if (bSuccess && statusCode != 0)
			{
				nlohmann::json jsonObject = nlohmann::json::parse(strBody);
				IPCapsResult result = jsonObject.get<IPCapsResult>();

				// NOTE: 6 can still be reported here... if we ONLY have ipV6 and no IPV4... proxies etc can cause / alter behavior.
				m_capIPv4 = (result.ipversion == 4) ? ECapabilityState::SUPPORTED : ECapabilityState::UNSUPPORTED;

				if (result.ipversion == 6)
				{
					m_capIPv6 = ECapabilityState::SUPPORTED;
				}
			}
			else
			{
				// only if it didnt resolve earlier
				if (m_capIPv4 != ECapabilityState::SUPPORTED)
				{
					m_capIPv4 = ECapabilityState::UNSUPPORTED;
				}
			}

			// now check IPv6
			std::map<std::string, std::string> mapHeaders;
			mapHeaders["AUTH_TOKEN"] = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetAuthToken();
			NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendGETRequest(std::format("https://playgenerals.online/cloud/env:dev:{}/DetermineIPCapabilities", NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetAuthToken()).c_str(), EIPProtocolVersion::FORCE_IPV6, mapHeaders, [=](bool bSuccess, int statusCode, std::string strBody)
				{
					if (bSuccess && statusCode != 0)
					{
						nlohmann::json jsonObject = nlohmann::json::parse(strBody);
						IPCapsResult result = jsonObject.get<IPCapsResult>();

						m_capIPv6 = (result.ipversion == 6) ? ECapabilityState::SUPPORTED : ECapabilityState::UNSUPPORTED;

						// NOTE: 4 can still be reported here... if we ONLY have ipV4 and no IPV6... proxies etc can cause / alter behavior.
						if (result.ipversion == 4)
						{
							m_capIPv4 = ECapabilityState::SUPPORTED;
						}
					}
					else
					{
						// only if it didnt resolve earlier
						if (m_capIPv6 != ECapabilityState::SUPPORTED)
						{
							m_capIPv6 = ECapabilityState::UNSUPPORTED;
						}
					}

					// now call back
					callbackDeterminedCaps();
				});
		});

	
}

void PortMapper::TryForwardPreferredPorts()
{
	// clean up anything we had, might be a re-enter
	CleanupPorts();

	// TODO_NGMP: Better detection of if in use already
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 gen(seed);
	std::uniform_int_distribution<> dis(35000, 45000);

	m_PreferredPortInternal = 5000; // fixed, we'll randomize the ext port instead
	m_PreferredPortExternal = dis(gen);

	NetworkLog("PortMapper: Attempting to open ext port %d and forward to local port %d", m_PreferredPortExternal, m_PreferredPortInternal);

	// TODO_NGMP: If everything fails, try a new port?
	
	// TODO_NGMP: Handle state where we are too early
	// prefer UPnP over NAT-PMP if both available
	if (m_capUPnP == ECapabilityState::SUPPORTED)
	{
		NetworkLog("PortMapper: Using UPnP");

		bool bSuccess = ForwardPreferredPort_UPnP();
		// if failed, and natpmp is available, try that next
		if (!bSuccess)
		{
			if (m_capNATPMP == ECapabilityState::SUPPORTED)
			{
				NetworkLog("PortMapper: UPnP failed, local network supports NATPMP, trying NATPMP instead.");
				bool bSuccess = ForwardPreferredPort_NATPMP();
				if (!bSuccess)
				{
					NetworkLog("PortMapper: NATPMP fallback failed.");
				}
				else
				{
					NetworkLog("PortMapper: NATPMP fallback succeeded.");
				}
			}
			else
			{
				NetworkLog("PortMapper: UPnP failed, and the local network does not support NATPMP.");
			}
		}
		else
		{
			NetworkLog("PortMapper: UPnP was successful");
		}
	}
	else if (m_capNATPMP == ECapabilityState::SUPPORTED)
	{
		NetworkLog("PortMapper: Using NAT-PMP");

		bool bSuccess = ForwardPreferredPort_NATPMP();
		// if failed, nothing to do, we either didnt have upnp, or already tried it as our preference
		if (!bSuccess)
		{
			NetworkLog("PortMapper: NATPMP failed and we have no fallback.");
		}
		else
		{
			NetworkLog("PortMapper: NAT-PMP was successful");
		}
	}
}

void PortMapper::CleanupPorts()
{
	if (m_bHasPortOpenedViaUPNP)
	{
		RemovePortMapping_UPnP();
	}

	if (m_bHasPortOpenedViaNATPMP)
	{
		RemovePortMapping_NATPMP();
	}
}

// TODO_NGMP: remove port mappings on exit
bool PortMapper::ForwardPreferredPort_UPnP()
{
	int error = 0;
	struct UPNPDev* upnp_dev = upnpDiscover(2000, nullptr, nullptr, 0, 0, 2,&error);

	char lan_address[64];
	char wan_address[64];
	struct UPNPUrls upnp_urls;
	struct IGDdatas upnp_data;
	int status = UPNP_GetValidIGD(upnp_dev, &upnp_urls, &upnp_data, lan_address, sizeof(lan_address), wan_address, sizeof(wan_address));

	if (status == 1)
	{
		NetworkLog("PortMapper: UPnP gateway found");
	}
	else
	{
		NetworkLog("PortMapper: UPnP gateway not found (%d)", status);
		return false;
	}

	std::string strExternalPort = std::format("{}", m_PreferredPortExternal);
	std::string strInternalPort = std::format("{}", m_PreferredPortInternal);

	error = UPNP_AddPortMapping(
		upnp_urls.controlURL,
		upnp_data.first.servicetype,
		strExternalPort.c_str(),
		strInternalPort.c_str(),
		lan_address,
		"C&C Generals Online",
		"UDP",
		nullptr,
		"86400"); // 24 hours

	NetworkLog("PortMapper: UPnP Mapping added with result %d", error);

	// check our mapping was added correctly
	size_t index = 0;
	while (true)
	{
		char map_wan_port[200] = "";
		char map_lan_address[200] = "";
		char map_lan_port[200] = "";
		char map_protocol[200] = "";
		char map_description[200] = "";
		char map_mapping_enabled[200] = "";
		char map_remote_host[200] = "";
		char map_lease_duration[200] = ""; // original time

		error = UPNP_GetGenericPortMappingEntry(
			upnp_urls.controlURL,
			upnp_data.first.servicetype,
			std::to_string(index).c_str(),
			map_wan_port,
			map_lan_address,
			map_lan_port,
			map_protocol,
			map_description,
			map_mapping_enabled,
			map_remote_host,
			map_lease_duration);

		if (!error
			&& strcmp(map_wan_port, strExternalPort.c_str()) == 0
			&& strcmp(map_lan_address, lan_address) == 0
			&& strcmp(map_lan_port, strInternalPort.c_str()) == 0
			&& strcmp(map_protocol, "UDP") == 0
			&& strcmp(map_description, "C&C Generals Online") == 0
			)
		{
			NetworkLog("PortMapper: UPnP Mapping validated on router");
			m_bHasPortOpenedViaUPNP = true;

			return true;
		}

		if (error)
		{
			break; // no more port mappings available
		}

		++index;
	}

	// NOTE: dont hard fail here. not finding an exact match might be OK, some routers mangle data etc
	NetworkLog("PortMapper: UPnP Mapping was not validated on router");
	m_bHasPortOpenedViaUPNP = true;

	return true;
}

bool PortMapper::ForwardPreferredPort_NATPMP()
{
	int r;
	natpmp_t natpmp;
	natpmpresp_t response;
	initnatpmp(&natpmp, 0, 0);

	sendnewportmappingrequest(&natpmp, NATPMP_PROTOCOL_UDP, m_PreferredPortInternal, m_PreferredPortExternal, 86400);
	do
	{
		fd_set fds;
		struct timeval timeout;
		FD_ZERO(&fds);
		FD_SET(natpmp.s, &fds);
		getnatpmprequesttimeout(&natpmp, &timeout);
		select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
		r = readnatpmpresponseorretry(&natpmp, &response);
	}
	while (r == NATPMP_TRYAGAIN);

	NetworkLog("PortMapper: NAT-PMP mapped external port %hu to internal port %hu with lifetime %u",
		response.pnu.newportmapping.mappedpublicport,
		response.pnu.newportmapping.privateport,
		response.pnu.newportmapping.lifetime);
	closenatpmp(&natpmp);

	m_bHasPortOpenedViaNATPMP = r >= 0;

	return m_bHasPortOpenedViaNATPMP;

}

void PortMapper::UPnP_RemoveAllMappingsToThisMachine()
{
	NetworkLog("PortMapper: UPnP unmapping all mappings to this machine");
	int error = 0;
	struct UPNPDev* upnp_dev = upnpDiscover(2000, nullptr, nullptr, 0, 0, 2, &error);

	char lan_address[64];
	char wan_address[64];
	struct UPNPUrls upnp_urls;
	struct IGDdatas upnp_data;
	int status = UPNP_GetValidIGD(upnp_dev, &upnp_urls, &upnp_data, lan_address, sizeof(lan_address), wan_address, sizeof(wan_address));

	if (status == 1)
	{
		NetworkLog("PortMapper: UPnP gateway found");
	}
	else
	{
		NetworkLog("PortMapper: UPnP gateway not found (%d)", status);
		return;
	}

	size_t index = 0;
	while (true)
	{
		char map_wan_port[200] = "";
		char map_lan_address[200] = "";
		char map_lan_port[200] = "";
		char map_protocol[200] = "";
		char map_description[200] = "";
		char map_mapping_enabled[200] = "";
		char map_remote_host[200] = "";
		char map_lease_duration[200] = ""; // original time

		error = UPNP_GetGenericPortMappingEntry(
			upnp_urls.controlURL,
			upnp_data.first.servicetype,
			std::to_string(index).c_str(),
			map_wan_port,
			map_lan_address,
			map_lan_port,
			map_protocol,
			map_description,
			map_mapping_enabled,
			map_remote_host,
			map_lease_duration);

		if (!error
			&& strcmp(map_lan_address, lan_address) == 0
			&& strcmp(map_protocol, "UDP") == 0
			&& strcmp(map_description, "C&C Generals Online") == 0
			)
		{
			NetworkLog("PortMapper: UPnP mass remove, Found a mapping, removing it");
			
			error = UPNP_DeletePortMapping(
				upnp_urls.controlURL,
				upnp_data.first.servicetype,
				map_wan_port,
				"UDP",
				0);

			if (error != UPNPCOMMAND_SUCCESS)
			{
				NetworkLog("PortMapper: UPnP mass remove remove of port failed");
			}
			else
			{
				NetworkLog("PortMapper: UPnP mass remove of port succeeded");
			}
		}

		if (error)
		{
			break; // no more port mappings available
		}

		++index;
	}
}

void PortMapper::RemovePortMapping_UPnP()
{
	NetworkLog("PortMapper: UPnP starting unmapping of port");
	int error = 0;
	struct UPNPDev* upnp_dev = upnpDiscover(2000, nullptr, nullptr, 0, 0, 2, &error);

	char lan_address[64];
	char wan_address[64];
	struct UPNPUrls upnp_urls;
	struct IGDdatas upnp_data;
	int status = UPNP_GetValidIGD(upnp_dev, &upnp_urls, &upnp_data, lan_address, sizeof(lan_address), wan_address, sizeof(wan_address));

	if (status == 1)
	{
		NetworkLog("PortMapper: UPnP gateway found");
	}
	else
	{
		NetworkLog("PortMapper: UPnP gateway not found (%d)", status);
		return;
	}

	std::string strExternalPort = std::format("{}", m_PreferredPortExternal);
	std::string strInternalPort = std::format("{}", m_PreferredPortInternal);

	error = UPNP_DeletePortMapping(
		upnp_urls.controlURL,
		upnp_data.first.servicetype,
		strExternalPort.c_str(),
		"UDP",
		0);

	if (error != UPNPCOMMAND_SUCCESS)
	{
		NetworkLog("PortMapper: UPnP unmapping of port failed");
	}
	else
	{
		NetworkLog("PortMapper: UPnP unmapping of port succeeded");
	}
}

void PortMapper::RemovePortMapping_NATPMP()
{
	NetworkLog("PortMapper: NAT-PMP starting unmapping of port");

	int r;
	natpmp_t natpmp;
	natpmpresp_t response;
	initnatpmp(&natpmp, 0, 0);

	sendnewportmappingrequest(&natpmp, NATPMP_PROTOCOL_UDP, m_PreferredPortInternal, m_PreferredPortExternal, 0);
	do
	{
		fd_set fds;
		struct timeval timeout;
		FD_ZERO(&fds);
		FD_SET(natpmp.s, &fds);
		getnatpmprequesttimeout(&natpmp, &timeout);
		select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
		r = readnatpmpresponseorretry(&natpmp, &response);
	} while (r == NATPMP_TRYAGAIN);

	if (r < 0)
	{
		NetworkLog("PortMapper: NAT-PMP unmapping of port failed");
	}
	else
	{
		NetworkLog("PortMapper: NAT-PMP unmapping of port succeeded");
	}
	closenatpmp(&natpmp);
}
