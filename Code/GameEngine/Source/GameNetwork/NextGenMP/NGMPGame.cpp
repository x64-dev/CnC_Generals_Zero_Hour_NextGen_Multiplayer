#include "GameNetwork/NextGenMP/NGMPGame.h"
#include "GameLogic/VictoryConditions.h"
#include "Common/PlayerList.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/FileTransfer.h"
#include "GameClient/MapUtil.h"
#include "GameClient/GameText.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "Common/RandomValue.h"
#include "GameNetwork/NextGenMP/NGMP_interfaces.h"
#include "GameNetwork/NetworkInterface.h"
#include "Common/GlobalData.h"

NGMPGameSlot::NGMPGameSlot()
{
	GameSlot();
	m_gameSpyLogin.clear();
	m_gameSpyLocale.clear();
	m_profileID = 0;
	m_wins = 0;
	m_losses = 0;
	m_rankPoints = 0;
	m_favoriteSide = 0;
	m_pingInt = 0;
	m_profileID = 0;
	m_pingStr.clear();
}

// NGMPGame ----------------------------------------

NGMPGame::NGMPGame()
{
	cleanUpSlotPointers();

	setLocalIP(0);
	//m_transport = NULL;

	m_localName = "localhost";

	m_ladderIP.clear();
	m_ladderPort = 0;

	enterGame(); // this is done on join in the GS impl, and must be called before setMap

	// NGMP: Store map
	setMap(NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->GetCurrentLobbyMapPath());

	// init
	//init();

	// NGMP: Populate slots
	UpdateSlotsFromCurrentLobby();
}


void NGMPGame::UpdateSlotsFromCurrentLobby()
{
	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		LobbyMember* pLobbyMember = NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->GetRoomMemberFromIndex(i);
		if (pLobbyMember != nullptr)
		{
			UnicodeString str;
			str.translate(pLobbyMember->m_strName.str());

			GameSlot* slot = getSlot(i);
			((NGMPGameSlot*)slot)->setState(SLOT_PLAYER, str, i);
		}

		// dont need to handle else here, we set it up upon lobby creation
	}
}


void NGMPGame::cleanUpSlotPointers(void)
{
	for (Int i = 0; i < MAX_SLOTS; ++i)
		setSlotPointer(i, &m_Slots[i]);
}

NGMPGameSlot* NGMPGame::getGameSpySlot(Int index)
{
	GameSlot* slot = getSlot(index);
	DEBUG_ASSERTCRASH(slot && (slot == &(m_Slots[index])), ("Bad game slot pointer\n"));
	return (NGMPGameSlot*)slot;
}

void NGMPGame::init(void)
{
	GameInfo::init();

	UpdateSlotsFromCurrentLobby();
}

void NGMPGame::setPingString(AsciiString pingStr)
{
	m_pingStr = pingStr;
	m_pingInt = 0;
	//m_pingInt = TheGameSpyInfo->getPingValue(pingStr);
}

Bool NGMPGame::amIHost(void) const
{
	return NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->IsHost();
}

void NGMPGame::resetAccepted(void)
{
	GameInfo::resetAccepted();

	if (amIHost())
	{
		/*
		peerStateChanged(TheGameSpyChat->getPeer());
		m_hasBeenQueried = false;
		DEBUG_LOG(("resetAccepted() called peerStateChange()\n"));
		*/
	}
}

Int NGMPGame::getLocalSlotNum(void) const
{
	DEBUG_ASSERTCRASH(m_inGame, ("Looking for local game slot while not in game"));
	if (!m_inGame)
		return -1;

	AsciiString localName = NGMP_OnlineServicesManager::GetInstance()->GetAuthInterface()->GetDisplayName();

	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		const GameSlot* slot = getConstSlot(i);
		if (slot == NULL) {
			continue;
		}
		if (slot->isPlayer(localName))
			return i;
	}
	return -1;
}

void NGMPGame::startGame(Int gameID)
{
	DEBUG_ASSERTCRASH(m_inGame, ("Starting a game while not in game"));
	DEBUG_LOG(("NGMPGame::startGame - game id = %d\n", gameID));
	//DEBUG_ASSERTCRASH(m_transport == NULL, ("m_transport is not NULL when it should be"));
	//DEBUG_ASSERTCRASH(TheNAT == NULL, ("TheNAT is not NULL when it should be"));

	//UnsignedInt localIP = TheGameSpyInfo->getInternalIP();
	UnsignedInt localIP = 0;
	setLocalIP(localIP);

	// fill in GS-specific info
	Int numHumans = 0;
	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		if (m_Slots[i].isHuman())
		{
			++numHumans;
			AsciiString gsName;
			gsName.translate(m_Slots[i].getName());
			m_Slots[i].setLoginName(gsName);

			if (m_isQM)
			{
				// TODO_NGMP: Does this matter anymore?
				if (getLocalSlotNum() == i)
					m_Slots[i].setProfileID(0);  // hehe - we know our own.  the rest, they'll tell us.
			}
			else
			{
				// TODO_NGMP
				/*
				PlayerInfoMap* pInfoMap = TheGameSpyInfo->getPlayerInfoMap();
				PlayerInfoMap::iterator it = pInfoMap->find(gsName);
				if (it != pInfoMap->end())
				{
					m_GameSpySlot[i].setProfileID(it->second.m_profileID);
					m_GameSpySlot[i].setLocale(it->second.m_locale);
					m_GameSpySlot[i].setSlotRankPoints(it->second.m_rankPoints);
					m_GameSpySlot[i].setFavoriteSide(it->second.m_side);
				}
				else
				{
					DEBUG_CRASH(("No player info for %s", gsName.str()));
				}
				*/
			}
		}
	}

	//#if defined(_DEBUG) || defined(_INTERNAL)
	if (numHumans < 2)
	{
		launchGame();
		
		
		// TODO_NGMP: LEave staging room? probably dont care anymore? its all one lobby nowadays
		//if (TheGameSpyInfo)
		//TheGameSpyInfo->leaveStagingRoom();
	}
	else
		//#endif defined(_DEBUG) || defined(_INTERNAL)
	{
		// TODO_NGMP: We dont care about this anymore? we're already connected
		//TheNAT = NEW NAT();
		//TheNAT->attachSlotList(m_slot, getLocalSlotNum(), m_localIP);
		//TheNAT->establishConnectionPaths();
	}
}

AsciiString NGMPGame::generateGameSpyGameResultsPacket(void)
{
	return AsciiString();
}

AsciiString NGMPGame::generateLadderGameResultsPacket(void)
{
	// TODO_NGMP
	AsciiString results;
	return results;
}

void NGMPGame::launchGame(void)
{
	setGameInProgress(TRUE);

	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		const NGMPGameSlot* slot = getGameSpySlot(i);
		if (slot->isHuman())
		{
			// TODO_NGMP:
			bool bPreordered = true;
			if (bPreordered)
				markPlayerAsPreorder(i);
		}
	}

	// Set up the game network
	AsciiString user;
	AsciiString userList;
	DEBUG_ASSERTCRASH(TheNetwork == NULL, ("For some reason TheNetwork isn't NULL at the start of this game.  Better look into that."));

	if (TheNetwork != NULL) {
		delete TheNetwork;
		TheNetwork = NULL;
	}

	// TODO_NGMP: do we care? we are already connected
	
	// Time to initialize TheNetwork for this game.
	TheNetwork = NetworkInterface::createNetwork();
	TheNetwork->init();

	// TODO_NGMP: Do we really care about these values anymore
	TheNetwork->setLocalAddress(getLocalIP(), 8888);

	NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->InitGameTransport();
	NextGenTransport* pTransport = NGMP_OnlineServicesManager::GetInstance()->GetLobbyInterface()->GetGameTransport();
	TheNetwork->attachTransport(pTransport);

	TheNetwork->parseUserList(this);

	if (TheGameLogic->isInGame()) {
		TheGameLogic->clearGameData();
	}

	Bool filesOk = DoAnyMapTransfers(this);

	// see if we really have the map.  if not, back out.
	TheMapCache->updateCache();
	if (!filesOk || TheMapCache->findMap(getMap()) == NULL)
	{
		DEBUG_LOG(("After transfer, we didn't really have the map.  Bailing...\n"));
		if (TheNetwork != NULL) {
			delete TheNetwork;
			TheNetwork = NULL;
		}
		GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:CouldNotTransferMap"));

		void PopBackToLobby(void);
		PopBackToLobby();
		return;
	}


	// shutdown the top, but do not pop it off the stack
//		TheShell->hideShell();
	// setup the Global Data with the Map and Seed
	TheWritableGlobalData->m_pendingFile = getMap();

	// send a message to the logic for a new game
	GameMessage* msg = TheMessageStream->appendMessage(GameMessage::MSG_NEW_GAME);
	msg->appendIntegerArgument(GAME_INTERNET);

	// TODO_NGMP
	//TheWritableGlobalData->m_useFpsLimit = false;

	// Set the random seed
	// TODO_NGMP: revisit this
	InitGameLogicRandom(getSeed());
	DEBUG_LOG(("InitGameLogicRandom( %d )\n", getSeed()));

	// mark us as "Loading" in the buddy list
	// TODO_NGMP
	/*
	BuddyRequest req;
	req.buddyRequestType = BuddyRequest::BUDDYREQUEST_SETSTATUS;
	req.arg.status.status = GP_PLAYING;
	strcpy(req.arg.status.statusString, "Loading");
	sprintf(req.arg.status.locationString, "%s", WideCharStringToMultiByte(TheGameSpyGame->getGameName().str()).c_str());
	TheGameSpyBuddyMessageQueue->addRequest(req);
	*/
}

void NGMPGame::reset(void)
{
	GameInfo::reset();
}