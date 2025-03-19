#pragma once

void NetworkLog(const char* fmt, ...);

// common game engine includes
#include "common/gsPlatformUtil.h"
#include "Common/UnicodeString.h"
#include "Common/AsciiString.h"

// standard libs
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>

// ngmp defines
#include "GameNetwork/NextGenMP/NextGenMP_defines.h"
#include "GameNetwork/NextGenMP/NetworkPacket.h"
#include "GameNetwork/NextGenMP/NetworkBitstream.h"
#include "GameNetwork/NextGenMP/NGMP_types.h"

// EOS
#include <eos_common.h>
#include <eos_p2p_types.h>
#include <eos_auth_types.h>
#include <eos_connect_types.h>
#include <eos_auth.h>
#include <eos_connect.h>
#include <eos_sdk.h>
#include <eos_p2p.h>
#include <eos_p2p_types.h>
#include <eos_types.h>
#include <eos_sessions.h>
#include <eos_lobby.h>
#include <eos_lobby_types.h>
#include <eos_metrics.h>
#include <eos_userinfo.h>
#include <eos_sanctions.h>
#include <eos_playerdatastorage.h>
#include <eos_playerdatastorage_types.h>
#include <eos_achievements.h>
#include <eos_logging.h>
#include <eos_titlestorage.h>
#include <eos_titlestorage_types.h>
#include <eos_friends.h>
#include <eos_friends_types.h>
#include <eos_stats.h>
#include <eos_stats_types.h>
#include <eos_reports.h>
#include <Windows/eos_windows.h>
#include <eos_integratedplatform_types.h>

// Steam
#include <steam_api.h>

// Devconsole
#include "../Console/Console.h"