#pragma once
#include "../GameInfo.h"
#include "eos_common.h"

class NGMPGameSlot : public GameSlot
{
public:
	NGMPGameSlot();
	Int getProfileID(void) const { return m_profileID; }
	void setProfileID(Int id) { m_profileID = id; }
	AsciiString getLoginName(void) const { return m_gameSpyLogin; }
	void setLoginName(AsciiString name) { m_gameSpyLogin = name; }
	AsciiString getLocale(void) const { return m_gameSpyLocale; }
	void setLocale(AsciiString name) { m_gameSpyLocale = name; }
	Int getWins(void) const { return m_wins; }
	Int getLosses(void) const { return m_losses; }
	void setWins(Int wins) { m_wins = wins; }
	void setLosses(Int losses) { m_losses = losses; }

	Int getSlotRankPoints(void) const { return m_rankPoints; }
	Int getFavoriteSide(void) const { return m_favoriteSide; }
	void setSlotRankPoints(Int val) { m_rankPoints = val; }
	void setFavoriteSide(Int val) { m_favoriteSide = val; }

	void setPingString(AsciiString pingStr) { m_pingStr = pingStr; }
	inline AsciiString getPingString(void) const { return m_pingStr; }
	inline Int getPingAsInt(void) const { return m_pingInt; }

	int64_t m_userID = -1;

protected:
	Int m_profileID;
	AsciiString m_gameSpyLogin;
	AsciiString m_gameSpyLocale;

	AsciiString m_pingStr;
	Int m_pingInt;
	Int m_wins, m_losses;
	Int m_rankPoints, m_favoriteSide;
};


class NGMPGame : public GameInfo
{
private:
	NGMPGameSlot m_Slots[MAX_SLOTS];
	UnicodeString m_gameName;
	Int m_id;
	//Transport* m_transport;
	AsciiString m_localName;
	Bool m_requiresPassword;
	Bool m_allowObservers;
	UnsignedInt m_version;
	UnsignedInt m_exeCRC;
	UnsignedInt m_iniCRC;
	Bool m_isQM;

	AsciiString m_ladderIP;
	AsciiString m_pingStr;
	Int m_pingInt;
	UnsignedShort m_ladderPort;

	Int m_reportedNumPlayers;
	Int m_reportedMaxPlayers;
	Int m_reportedNumObservers;

public:
	NGMPGame();
	virtual void reset(void);

	void UpdateSlotsFromCurrentLobby();

	void cleanUpSlotPointers(void);
	inline void setID(Int id) { m_id = id; }
	inline Int getID(void) const { return m_id; }

	inline void setHasPassword(Bool val) { m_requiresPassword = val; }
	inline Bool getHasPassword(void) const { return m_requiresPassword; }
	inline void setAllowObservers(Bool val) { m_allowObservers = val; }
	inline Bool getAllowObservers(void) const { return m_allowObservers; }

	inline void setVersion(UnsignedInt val) { m_version = val; }
	inline UnsignedInt getVersion(void) const { return m_version; }
	inline void setExeCRC(UnsignedInt val) { m_exeCRC = val; }
	inline UnsignedInt getExeCRC(void) const { return m_exeCRC; }
	inline void setIniCRC(UnsignedInt val) { m_iniCRC = val; }
	inline UnsignedInt getIniCRC(void) const { return m_iniCRC; }

	inline void setReportedNumPlayers(Int val) { m_reportedNumPlayers = val; }
	inline Int getReportedNumPlayers(void) const { return m_reportedNumPlayers; }

	inline void setReportedMaxPlayers(Int val) { m_reportedMaxPlayers = val; }
	inline Int getReportedMaxPlayers(void) const { return m_reportedMaxPlayers; }

	inline void setReportedNumObservers(Int val) { m_reportedNumObservers = val; }
	inline Int getReportedNumObservers(void) const { return m_reportedNumObservers; }

	inline void setLadderIP(AsciiString ladderIP) { m_ladderIP = ladderIP; }
	inline AsciiString getLadderIP(void) const { return m_ladderIP; }
	inline void setLadderPort(UnsignedShort ladderPort) { m_ladderPort = ladderPort; }
	inline UnsignedShort getLadderPort(void) const { return m_ladderPort; }
	void setPingString(AsciiString pingStr);
	inline AsciiString getPingString(void) const { return m_pingStr; }
	inline Int getPingAsInt(void) const { return m_pingInt; }

	virtual Bool amIHost(void) const;															///< Convenience function - is the local player the game host?

	NGMPGameSlot* getGameSpySlot(Int index);

	AsciiString generateGameSpyGameResultsPacket(void);
	AsciiString generateLadderGameResultsPacket(void);
	void markGameAsQM(void) { m_isQM = TRUE; }
	Bool isQMGame(void) { return m_isQM; }

	virtual void init(void);
	virtual void resetAccepted(void);															///< Reset the accepted flag on all players

	virtual void startGame(Int gameID);														///< Mark our game as started and record the game ID.
	void launchGame(void);																			///< NAT negotiation has finished - really start
	virtual Int getLocalSlotNum(void) const;										///< Get the local slot number, or -1 if we're not present

	inline void setGameName(UnicodeString name) { m_gameName = name; }
	inline UnicodeString getGameName(void) const { return m_gameName; }

	inline void setLocalName(AsciiString name) { m_localName = name; }
};