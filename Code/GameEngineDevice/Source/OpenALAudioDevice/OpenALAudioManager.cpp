#include "Lib/Basetype.h"
#include "OpenALAudioDevice/OpenALAudioManager.h"

#include "Common/AudioAffect.h"
#include "Common/AudioHandleSpecialValues.h"
#include "Common/AudioRequest.h"
#include "Common/AudioSettings.h"
#include "Common/AsciiString.h"
#include "Common/AudioEventInfo.h"
#include "Common/FileSystemEA.h"
#include "Common/GameCommon.h"
#include "Common/GameSounds.h"
#include "Common/CRCDebug.h"
#include "Common/GlobalData.h"

#include "GameClient/DebugDisplay.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/View.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/TerrainLogic.h"

#include "Common/File.h"

#include "OpenALAudioDevice/OpenALAudioLoader.h"

// Constructor
OpenALAudioManager::OpenALAudioManager()
    : m_digitalHandle(nullptr)
{
	device = nullptr;
	context = nullptr;
	m_digitalHandle = nullptr;

	// Initialize pool bookkeeping
	//for (int i = 0; i < NUM_POOLED_SOURCES; ++i)
	//{
	//	m_sourcePool[i] = 0;
	//	m_sourceInUse[i] = false;
	//}
	m_musicSource = 0;
}

// Destructor
OpenALAudioManager::~OpenALAudioManager()
{
    // TODO: Clean up any allocated resources.
}

#if defined(_DEBUG) || defined(_INTERNAL)
void OpenALAudioManager::audioDebugDisplay(DebugDisplayInterface* dd, void* data, FILE* fp)
{
    // TODO: Implement OpenAL debug display functionality.
}

AudioHandle OpenALAudioManager::addAudioEvent(const AudioEventRTS* eventToAdd)
{
	//if (TheGlobalData->m_preloadReport) {
	//	if (!eventToAdd->getEventName().isEmpty()) {
	//		m_allEventsLoaded.insert(eventToAdd->getEventName());
	//	}
	//}

	return AudioManager::addAudioEvent(eventToAdd);
}
#endif

//-------------------------------------------------------------------------------------------------
// AudioDevice functions
void OpenALAudioManager::init()
{
	AudioManager::init();

	m_selectedSpeakerType = TheAudio->translateSpeakerTypeToUnsignedInt(m_prefSpeaker);
	
	openDevice();

	// Now that we're all done, update the cached variables so that everything is in sync.
	TheAudio->refreshCachedVariables();
}

void OpenALAudioManager::postProcessLoad()
{
	AudioManager::postProcessLoad();
}

void OpenALAudioManager::reset()
{
	AudioManager::reset();
	stopAllAudioImmediately();
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::stopAllAudioImmediately(void)
{
	std::list<OpenALPlayingAudio*>::iterator it;
	OpenALPlayingAudio* playing;

	if (m_playingSounds.size() > 0)
	{
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ) {
			playing = *it;
			if (!playing) {
				continue;
			}

			releasePlayingAudio(playing);
			it = m_playingSounds.erase(it);
		}
	}
	
	if (m_playing3DSounds.size() > 0)
	{
		for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ) {
			playing = *it;
			if (!playing) {
				continue;
			}

			releasePlayingAudio(playing);
			it = m_playing3DSounds.erase(it);
		}
	}
	
	if (m_playingStreams.size() > 0)
	{
		for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ) {
			playing = (*it);
			if (!playing) {
				continue;
			}

			releasePlayingAudio(playing);
			it = m_playingStreams.erase(it);
		}
	}
	

	//std::list<HAUDIO>::iterator hit;
	//for (hit = m_audioForcePlayed.begin(); hit != m_audioForcePlayed.end(); ++hit) {
	//	if (*hit) {
	//		AIL_quick_unload(*hit);
	//	}
	//}
	//m_audioForcePlayed.clear();
}

void OpenALAudioManager::nextMusicTrack(void)
{
	AsciiString trackName;
	std::list<OpenALPlayingAudio*>::iterator it;
	OpenALPlayingAudio* playing;
	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			trackName = playing->m_audioEventRTS->getEventName();
		}
	}

	// Stop currently playing music
	TheAudio->removeAudioEvent(AHSV_StopTheMusic);

	trackName = nextTrackName(trackName);
	AudioEventRTS newTrack(trackName);
	TheAudio->addAudioEvent(&newTrack);
}

void OpenALAudioManager::prevMusicTrack(void)
{
	AsciiString trackName;
	std::list<OpenALPlayingAudio*>::iterator it;
	OpenALPlayingAudio* playing;
	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			trackName = playing->m_audioEventRTS->getEventName();
		}
	}

	// Stop currently playing music 
	TheAudio->removeAudioEvent(AHSV_StopTheMusic);

	trackName = prevTrackName(trackName);
	AudioEventRTS newTrack(trackName);
	TheAudio->addAudioEvent(&newTrack);
}

Bool OpenALAudioManager::isMusicPlaying(void) const
{
	std::list<OpenALPlayingAudio*>::const_iterator it;
	OpenALPlayingAudio* playing;
	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			return TRUE;
		}
	}

	return FALSE;
}

AsciiString OpenALAudioManager::getMusicTrackName(void) const
{
	// First check the requests. If there's one there, then report that as the currently playing track.
	std::list<AudioRequest*>::const_iterator ait;
	for (ait = m_audioRequests.begin(); ait != m_audioRequests.end(); ++ait) {
		if ((*ait)->m_request != AR_Play) {
			continue;
		}

		if (!(*ait)->m_usePendingEvent) {
			continue;
		}

		if ((*ait)->m_pendingEvent->getAudioEventInfo()->m_soundType == AT_Music) {
			return (*ait)->m_pendingEvent->getEventName();
		}
	}

	std::list<OpenALPlayingAudio*>::const_iterator it;
	OpenALPlayingAudio* playing;
	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			return playing->m_audioEventRTS->getEventName();
		}
	}

	return AsciiString::TheEmptyString;
}



void* OpenALAudioManager::getDevice(void)
{
    return m_digitalHandle;
}


void OpenALAudioManager::pauseAmbient(Bool shouldPause)
{
    
}

void OpenALAudioManager::stopAllAmbientsBy(Object* objID)
{

}

void OpenALAudioManager::stopAllAmbientsBy(Drawable* drawID)
{

}

void OpenALAudioManager::killAudioEventImmediately(AudioHandle audioEvent)
{
	//First look for it in the request list.
	std::list<AudioRequest*>::iterator ait;
	for (ait = m_audioRequests.begin(); ait != m_audioRequests.end(); ait++)
	{
		AudioRequest* req = (*ait);
		if (req && req->m_request == AR_Play && req->m_handleToInteractOn == audioEvent)
		{
			req->deleteInstance();
			ait = m_audioRequests.erase(ait);
			return;
		}
	}

	//Look for matching 3D sound to kill
	std::list<OpenALPlayingAudio*>::iterator it;
	for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); it++)
	{
		OpenALPlayingAudio* audio = (*it);
		if (!audio)
		{
			continue;
		}

		if (audio->m_audioEventRTS->getPlayingHandle() == audioEvent)
		{
			releasePlayingAudio(audio);
			m_playing3DSounds.erase(it);
			return;
		}
	}

	//Look for matching 2D sound to kill
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); it++)
	{
		OpenALPlayingAudio* audio = (*it);
		if (!audio)
		{
			continue;
		}

		if (audio->m_audioEventRTS->getPlayingHandle() == audioEvent)
		{
			releasePlayingAudio(audio);
			m_playingSounds.erase(it);
			return;
		}
	}

	//Look for matching steaming sound to kill
	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); it++)
	{
		OpenALPlayingAudio* audio = (*it);
		if (!audio)
		{
			continue;
		}

		if (audio->m_audioEventRTS->getPlayingHandle() == audioEvent)
		{
			releasePlayingAudio(audio);
			m_playingStreams.erase(it);
			return;
		}
	}
}


Bool OpenALAudioManager::isCurrentlyPlaying(AudioHandle handle)
{
	std::list<OpenALPlayingAudio*>::iterator it;
	OpenALPlayingAudio* playing;

	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS->getPlayingHandle() == handle) {
			return true;
		}
	}

	for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS->getPlayingHandle() == handle) {
			return true;
		}
	}

	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS->getPlayingHandle() == handle) {
			return true;
		}
	}

	return false;
}

void OpenALAudioManager::notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags)
{
    // TODO: Handle notification that an audio event has completed.
}

UnsignedInt OpenALAudioManager::getProviderCount(void) const
{
    return 1;
}

AsciiString OpenALAudioManager::getProviderName(UnsignedInt providerNum) const
{    
    return "OpenAL";
}

UnsignedInt OpenALAudioManager::getProviderIndex(AsciiString providerName) const
{
    return 0;
}

void OpenALAudioManager::selectProvider(UnsignedInt providerNdx)
{
}

void OpenALAudioManager::unselectProvider(void)
{
    // TODO: Unselect the current audio provider.
}

UnsignedInt OpenALAudioManager::getSelectedProvider(void) const
{
    // TODO: Return the index of the currently selected provider.
    return 0;
}


//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::setSpeakerType(UnsignedInt speakerType)
{
	m_selectedSpeakerType = speakerType;
}

//-------------------------------------------------------------------------------------------------
UnsignedInt OpenALAudioManager::getSpeakerType(void)
{
	return m_selectedSpeakerType;
}

void* OpenALAudioManager::getHandleForBink(void)
{
    // TODO: Return the handle for Bink if applicable.
    return nullptr;
}

void OpenALAudioManager::releaseHandleForBink(void)
{
    // TODO: Release the Bink handle.
}

void OpenALAudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay)
{
	if (!eventToPlay->getAudioEventInfo()) {
		getInfoForAudioEvent(eventToPlay);
		if (!eventToPlay->getAudioEventInfo()) {
			DEBUG_CRASH(("No info for forced audio event '%s'\n", eventToPlay->getEventName().str()));
			return;
		}
	}

	switch (eventToPlay->getAudioEventInfo()->m_soundType)
	{
	case AT_Music:
		if (!isOn(AudioAffect_Music))
			return;
		break;
	case AT_SoundEffect:
		if (!isOn(AudioAffect_Sound) || !isOn(AudioAffect_Sound3D))
			return;
		break;
	case AT_Streaming:
		if (!isOn(AudioAffect_Speech))
			return;
		break;
	}

	AudioEventRTS event = *eventToPlay;

	event.generateFilename();
	event.generatePlayInfo();

	std::list<std::pair<AsciiString, Real> >::iterator it;
	for (it = m_adjustedVolumes.begin(); it != m_adjustedVolumes.end(); ++it) {
		if (it->first == event.getEventName()) {
			event.setVolume(it->second);
			break;
		}
	}

	AsciiString fileToPlay = event.getFilename();
	OpenALPlayingAudio* audio = allocatePlayingAudio();

	audio->m_audioEventRTS = (AudioEventRTS*)eventToPlay;
	// audio->source = something <-- fix this.
	audio->buffer = NULL;
	audio->m_type = PAT_Sample;

	playSample((AudioEventRTS * )eventToPlay, audio, false);
	if (audio->source != 0)
	{
		m_playingSounds.push_back(audio);
		m_sound->notifyOf2DSampleStart();

		audio = NULL;
	}
	else
	{
		releasePlayingAudio(audio);
	}
}

Bool OpenALAudioManager::doesViolateLimit(AudioEventRTS* event) const
{
	Int limit = event->getAudioEventInfo()->m_limit;
	if (limit == 0) {
		return false;
	}

	Int totalCount = 0;
	Int totalRequestCount = 0;

	std::list<OpenALPlayingAudio*>::const_iterator it;
	if (!event->isPositionalAudio()) {
		// 2-D
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
			if ((*it)->m_audioEventRTS->getEventName() == event->getEventName()) {
				if (totalCount == 0) {
					// This is the oldest audio of this type playing.
					event->setHandleToKill((*it)->m_audioEventRTS->getPlayingHandle());
				}
				++totalCount;
			}
		}
	}
	else {
		// 3-D
		for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it) {
			if ((*it)->m_audioEventRTS->getEventName() == event->getEventName()) {
				if (totalCount == 0) {
					// This is the oldest audio of this type playing.
					event->setHandleToKill((*it)->m_audioEventRTS->getPlayingHandle());
				}
				++totalCount;
			}
		}
	}

	// Also check the request list in case we've requested to play this sound.
	std::list<AudioRequest*>::const_iterator arIt;
	for (arIt = m_audioRequests.begin(); arIt != m_audioRequests.end(); ++arIt) {
		AudioRequest* req = (*arIt);
		if (req == NULL) {
			continue;
		}
		if (req->m_usePendingEvent)
		{
			if (req->m_pendingEvent->getEventName() == event->getEventName())
			{
				totalRequestCount++;
				totalCount++;
			}
		}
	}

	//If our event is an interrupting type, then normally we would always add it. The exception is when we have requested
	//multiple sounds in the same frame and those requests violate the limit. Because we don't have any "old" sounds to
	//remove in the case of an interrupt, we need to catch it early and prevent the sound from being added if we already
	//reached the limit
	if (event->getAudioEventInfo()->m_control & AC_INTERRUPT)
	{
		if (totalRequestCount < limit)
		{
			Int totalPlayingCount = totalCount - totalRequestCount;
			if (totalRequestCount + totalPlayingCount < limit)
			{
				//We aren't exceeding the actual limit, then clear the kill handle.
				event->setHandleToKill(0);
				return false;
			}

			//We are exceeding the limit - the kill handle will kill the
			//oldest playing sound to enforce the actual limit.
			return false;
		}
	}

	if (totalCount < limit)
	{
		event->setHandleToKill(0);
		return false;
	}

	return true;
}

Bool OpenALAudioManager::isPlayingLowerPriority(AudioEventRTS* event) const
{
	//We don't actually want to do anything to this CONST function. Remember, we're
	//just checking to see if there is a lower priority sound.
	AudioPriority priority = event->getAudioEventInfo()->m_priority;
	if (priority == AP_LOWEST)
	{
		//If the event we pass in is the lowest priority, don't bother checking because
		//there is nothing lower priority than lowest.
		return false;
	}
	std::list<OpenALPlayingAudio*>::const_iterator it;
	if (!event->isPositionalAudio()) {
		// 2-D
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
			if ((*it)->m_audioEventRTS->getAudioEventInfo()->m_priority < priority) {
				//event->setHandleToKill((*it)->m_audioEventRTS->getPlayingHandle());
				return true;
			}
		}
	}
	else {
		// 3-D
		for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it) {
			if ((*it)->m_audioEventRTS->getAudioEventInfo()->m_priority < priority) {
				//event->setHandleToKill((*it)->m_audioEventRTS->getPlayingHandle());
				return true;
			}
		}
	}

	return false;
}

Bool OpenALAudioManager::isPlayingAlready(AudioEventRTS* event) const
{
	std::list<OpenALPlayingAudio*>::const_iterator it;
	if (!event->isPositionalAudio()) {
		// 2-D
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
			if ((*it)->m_audioEventRTS->getEventName() == event->getEventName()) {
				return true;
			}
		}
	}
	else {
		// 3-D
		for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it) {
			if ((*it)->m_audioEventRTS->getEventName() == event->getEventName()) {
				return true;
			}
		}
	}

	return false;
}

Bool OpenALAudioManager::isObjectPlayingVoice(UnsignedInt objID) const
{
	if (objID == 0) {
		return false;
	}

	std::list<OpenALPlayingAudio*>::const_iterator it;
	// 2-D
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		if ((*it)->m_audioEventRTS->getObjectID() == objID && (*it)->m_audioEventRTS->getAudioEventInfo()->m_type & ST_VOICE) {
			return true;
		}
	}

	// 3-D
	for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it) {
		if ((*it)->m_audioEventRTS->getObjectID() == objID && (*it)->m_audioEventRTS->getAudioEventInfo()->m_type & ST_VOICE) {
			return true;
		}
	}

	return false;
}

Bool OpenALAudioManager::killLowestPrioritySoundImmediately(AudioEventRTS* event)
{
	//Actually, we want to kill the LOWEST PRIORITY SOUND, not the first "lower" priority
	//sound we find, because it could easily be 
	AudioEventRTS* lowestPriorityEvent = findLowestPrioritySound(event);
	if (lowestPriorityEvent)
	{
		std::list<OpenALPlayingAudio*>::iterator it;
		if (event->isPositionalAudio())
		{
			for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it)
			{
				OpenALPlayingAudio* playing = (*it);
				if (!playing)
				{
					continue;
				}

				if (playing->m_audioEventRTS && playing->m_audioEventRTS == lowestPriorityEvent)
				{
					//Release this 3D sound channel immediately because we are going to play another sound in it's place.
					releasePlayingAudio(playing);
					m_playing3DSounds.erase(it);
					return TRUE;
				}
			}
		}
		else
		{
			for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it)
			{
				OpenALPlayingAudio* playing = (*it);
				if (!playing)
				{
					continue;
				}

				if (playing->m_audioEventRTS && playing->m_audioEventRTS == lowestPriorityEvent)
				{
					//Release this 3D sound channel immediately because we are going to play another sound in it's place.
					releasePlayingAudio(playing);
					m_playingSounds.erase(it);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

AudioEventRTS* OpenALAudioManager::findLowestPrioritySound(AudioEventRTS* event)
{
    // TODO: Find and return the lowest priority sound.
    return nullptr;
}

void OpenALAudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume)
{
    // TODO: Adjust the volume of the specified audio event.
}

void OpenALAudioManager::removePlayingAudio(AsciiString eventName)
{
	std::list<OpenALPlayingAudio*>::iterator it;

	OpenALPlayingAudio* playing = NULL;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); )
	{
		playing = *it;
		if (playing && playing->m_audioEventRTS->getEventName() == eventName)
		{
			releasePlayingAudio(playing);
			it = m_playingSounds.erase(it);
		}
		else
		{
			it++;
		}
	}

	for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); )
	{
		playing = *it;
		if (playing && playing->m_audioEventRTS->getEventName() == eventName)
		{
			releasePlayingAudio(playing);
			it = m_playing3DSounds.erase(it);
		}
		else
		{
			it++;
		}
	}

	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); )
	{
		playing = *it;
		if (playing && playing->m_audioEventRTS->getEventName() == eventName)
		{
			releasePlayingAudio(playing);
			it = m_playingStreams.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void OpenALAudioManager::removeAllDisabledAudio()
{
    // TODO: Remove all disabled audio events.
}

OpenALPlayingAudio* OpenALAudioManager::allocatePlayingAudio(void)
{
    OpenALPlayingAudio* aud = NEW OpenALPlayingAudio;	// poolify
    aud->m_status = PS_Playing;
    return aud;
}


ALuint OpenALAudioManager::openFile(AudioEventRTS* eventToOpenFrom)
{

    AsciiString strToFind;
    switch (eventToOpenFrom->getNextPlayPortion())
    {
    case PP_Attack:
        strToFind = eventToOpenFrom->getAttackFilename();
        break;
    case PP_Sound:
        strToFind = eventToOpenFrom->getFilename();
        break;
    case PP_Decay:
        strToFind = eventToOpenFrom->getDecayFilename();
        break;
    case PP_Done:
        return NULL;
    }

    return OpenALAudioLoader::loadFromFile(strToFind.str());
}


//-------------------------------------------------------------------------------------------------
const Coord3D* OpenALAudioManager::getCurrentPositionFromEvent(AudioEventRTS* event)
{
    if (!event->isPositionalAudio()) {
        return NULL;
    }

    return event->getCurrentPosition();
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::stopAllSpeech(void)
{
	std::list<OpenALPlayingAudio*>::iterator it;
	OpenALPlayingAudio* playing;
	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ) {
		playing = (*it);
		if (!playing) {
			continue;
		}

		if (playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Streaming) {
			releasePlayingAudio(playing);
			it = m_playingStreams.erase(it);
		}
		else {
			++it;
		}
	}
}

//-------------------------------------------------------------------------------------------------

void OpenALAudioManager::playAudioEvent(AudioEventRTS* event) 
{
    const AudioEventInfo* info = event->getAudioEventInfo();
    if (!info) {
        return;
    }

    std::list<OpenALPlayingAudio*>::iterator it;
    OpenALPlayingAudio* playing = NULL;

    AudioHandle handleToKill = event->getHandleToKill();

    AsciiString fileToPlay = event->getFilename();
    OpenALPlayingAudio* audio = allocatePlayingAudio();
	switch (info->m_soundType)
	{
	case AT_Music:
	case AT_Streaming:
	{
#ifdef INTENSIVE_AUDIO_DEBUG
		DEBUG_LOG(("- Stream\n"));
#endif

		if ((info->m_soundType == AT_Streaming) && event->getUninterruptable()) {
			stopAllSpeech();
		}

		Real curVolume = 1.0;
		if (info->m_soundType == AT_Music) {
			curVolume = m_musicVolume;
		}
		else {
			curVolume = m_speechVolume;
		}
		curVolume *= event->getVolume();

		Bool foundSoundToReplace = false;
		if (handleToKill) {
			for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
				playing = (*it);
				if (!playing) {
					continue;
				}

				if (playing->m_audioEventRTS && playing->m_audioEventRTS->getPlayingHandle() == handleToKill)
				{
					//Release this streaming channel immediately because we are going to play another sound in it's place.
					releasePlayingAudio(playing);
					m_playingStreams.erase(it);
					foundSoundToReplace = true;
					break;
				}
			}
		}

		if (info->m_soundType == AT_Music)
		{
			// audio->source = something <-- fix this.
			audio->m_audioEventRTS = event;
			audio->m_type = PAT_Stream;

			playSample(event, audio, true);
			if (audio->source != 0)
			{
				if ((info->m_soundType == AT_Streaming) && event->getUninterruptable()) {
					setDisallowSpeech(TRUE);
				}
				//AIL_set_stream_volume_pan(stream, curVolume, 0.5f);
				//playStream(event, stream);			
				alSourcef(audio->source, AL_GAIN, curVolume);

				m_playingStreams.push_back(audio);
				audio = NULL;
			}
		}
		else
		{
			// Push it onto the list of playing things
			audio->m_audioEventRTS = event;
			// audio->source = something <-- fix this.
			audio->buffer = NULL;
			audio->m_type = PAT_Sample;

			playSample(event, audio, false);
			if (audio->source != 0)
			{
				m_playingSounds.push_back(audio);
				m_sound->notifyOf2DSampleStart();

				audio = NULL;
			}
		}
		break;
	}

	case AT_SoundEffect:
	{
#ifdef INTENSIVE_AUDIO_DEBUG
		DEBUG_LOG(("- Sound"));
#endif


		if (event->isPositionalAudio()) {
			// Sounds that are non-global are positional 3-D sounds. Deal with them accordingly
#ifdef INTENSIVE_AUDIO_DEBUG
			DEBUG_LOG((" Positional"));
#endif
			Bool foundSoundToReplace = false;
			if (handleToKill)
			{
				for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it) {
					playing = (*it);
					if (!playing) {
						continue;
					}

					if (playing->m_audioEventRTS && playing->m_audioEventRTS->getPlayingHandle() == handleToKill)
					{
						//Release this 3D sound channel immediately because we are going to play another sound in it's place.
						releasePlayingAudio(playing);
						m_playing3DSounds.erase(it);
						foundSoundToReplace = true;
						break;
					}
				}
			}

			if (!handleToKill || foundSoundToReplace)
			{
				// Push it onto the list of playing things
				audio->m_audioEventRTS = event;
				// audio->source = something <-- fix this.
				audio->buffer = 0;
				audio->m_type = PAT_3DSample;

				playSample3D(event, audio);
			}
					
			if(audio->source != 0)
			{
				m_playing3DSounds.push_back(audio);
				m_sound->notifyOf3DSampleStart();
				audio = NULL;
#ifdef INTENSIVE_AUDIO_DEBUG
				DEBUG_LOG((" Playing.\n"));
#endif
			}
		}
		else
		{
			// UI sounds are always 2-D. All other sounds should be Positional
			// Unit acknowledgement, etc, falls into the UI category of sound.
			Bool foundSoundToReplace = false;
			if (handleToKill) {
				for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
					playing = (*it);
					if (!playing) {
						continue;
					}

					if (playing->m_audioEventRTS && playing->m_audioEventRTS->getPlayingHandle() == handleToKill)
					{
						//Release this 2D sound channel immediately because we are going to play another sound in it's place.
						releasePlayingAudio(playing);
						m_playingSounds.erase(it);
						foundSoundToReplace = true;
						break;
					}
				}
			}

			if (!handleToKill || foundSoundToReplace)
			{
				// Push it onto the list of playing things
				audio->m_audioEventRTS = event;
				// audio->source = something <-- fix this.
				audio->buffer = NULL;
				audio->m_type = PAT_Sample;

				playSample(event, audio, false);
			}

			if (audio->source != 0)
			{
				m_playingSounds.push_back(audio);				
				m_sound->notifyOf2DSampleStart();

				audio = NULL;
			}

#ifdef INTENSIVE_AUDIO_DEBUG
			DEBUG_LOG((" Playing.\n"));
#endif
		}
		break;
	}
	}

    // If we were able to successfully play audio, then we set it to NULL above. (And it will be freed
    // later. However, if audio is non-NULL at this point, then it must be freed.
    if (audio) {
        releasePlayingAudio(audio);
    }
}

void OpenALAudioManager::stopAudioEvent(AudioHandle handle) 
{
#ifdef INTENSIVE_AUDIO_DEBUG
	DEBUG_LOG(("MILES (%d) - Processing stop request: %d\n", TheGameLogic->getFrame(), handle));
#endif

	std::list<OpenALPlayingAudio*>::iterator it;
	if (handle == AHSV_StopTheMusic || handle == AHSV_StopTheMusicFade) {
		// for music, just find the currently playing music stream and kill it.
		for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
			OpenALPlayingAudio* audio = (*it);
			if (!audio) {
				continue;
			}

			if (audio->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music)
			{
				if (handle == AHSV_StopTheMusicFade)
				{
					m_fadingAudio.push_back(audio);
				}
				else
				{
					//m_stoppedAudio.push_back(audio);
					releasePlayingAudio(audio);
				}
				m_playingStreams.erase(it);
				break;
			}
		}
	}

	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
		OpenALPlayingAudio* audio = (*it);
		if (!audio) {
			continue;
		}

		if (audio->m_audioEventRTS->getPlayingHandle() == handle) {
			// found it
			audio->m_requestStop = true;
			notifyOfAudioCompletion((UnsignedInt)(audio->source), PAT_Stream);
			break;
		}
	}

	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		OpenALPlayingAudio* audio = (*it);
		if (!audio) {
			continue;
		}

		if (audio->m_audioEventRTS->getPlayingHandle() == handle) {
			audio->m_requestStop = true;
			break;
		}
	}

	for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it) {
		OpenALPlayingAudio* audio = (*it);
		if (!audio) {
			continue;
		}

		if (audio->m_audioEventRTS->getPlayingHandle() == handle) {
#ifdef INTENSIVE_AUDIO_DEBUG
			DEBUG_LOG((" (%s)\n", audio->m_audioEventRTS->getEventName()));
#endif
			audio->m_requestStop = true;
			break;
		}
	}
}

void OpenALAudioManager::pauseAudioEvent(AudioHandle handle) 
{

}

void OpenALAudioManager::processRequest(AudioRequest* req)
{
    switch (req->m_request)
    {
        case AR_Play:
        {
            playAudioEvent(req->m_pendingEvent);
            break;
        }
        case AR_Pause:
        {
            pauseAudioEvent(req->m_handleToInteractOn);
            break;
        }
        case AR_Stop:
        {
            stopAudioEvent(req->m_handleToInteractOn);
            break;
        }
    }
}

void OpenALAudioManager::processRequestList(void)
{
    std::list<AudioRequest*>::iterator it;
    for (it = m_audioRequests.begin(); it != m_audioRequests.end(); /* empty */) {
        AudioRequest* req = (*it);
        if (req == NULL) {
            continue;
        }

        if (!shouldProcessRequestThisFrame(req)) {
            adjustRequest(req);
            ++it;
            continue;
        }

        if (!req->m_requiresCheckForSample || checkForSample(req)) {
            processRequest(req);
        }
        req->deleteInstance();
        it = m_audioRequests.erase(it);
    }
}

Bool OpenALAudioManager::shouldProcessRequestThisFrame(AudioRequest* req) const
{
	if (!req->m_usePendingEvent) {
		return true;
	}

	if (req->m_pendingEvent->getDelay() < MSEC_PER_LOGICFRAME_REAL) {
		return true;
	}

	return false;
}

void OpenALAudioManager::adjustRequest(AudioRequest* req)
{
	if (!req->m_usePendingEvent) {
		return;
	}

	req->m_pendingEvent->decrementDelay(MSEC_PER_LOGICFRAME_REAL);
	req->m_requiresCheckForSample = true;
}

Bool OpenALAudioManager::checkForSample(AudioRequest* req)
{
	if (!req->m_usePendingEvent) {
		return true;
	}

	if (req->m_pendingEvent->getAudioEventInfo()->m_type != AT_SoundEffect) {
		return true;
	}

	return m_sound->canPlayNow(req->m_pendingEvent);
}

void OpenALAudioManager::setHardwareAccelerated(Bool accel)
{
    // TODO: Set hardware acceleration for audio.
}

void OpenALAudioManager::setSpeakerSurround(Bool surround)
{
    // TODO: Set surround speaker configuration.
}

Real OpenALAudioManager::getFileLengthMS(AsciiString strToLoad) const
{
    // TODO: Return the length of the specified file in milliseconds.
    return 0.0;
}

void OpenALAudioManager::closeAnySamplesUsingFile(const void* fileToClose)
{
	std::list<OpenALPlayingAudio*>::iterator it;
	OpenALPlayingAudio* playing;

	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ) {
		playing = *it;
		if (!playing) {
			continue;
		}

		if (playing->source == (ALuint)fileToClose) {
			releasePlayingAudio(playing);
			it = m_playingSounds.erase(it);
		}
		else {
			++it;
		}
	}

	for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ) {
		playing = *it;
		if (!playing) {
			continue;
		}

		if (playing->source == (ALuint)fileToClose) {
			releasePlayingAudio(playing);
			it = m_playing3DSounds.erase(it);
		}
		else {
			++it;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::adjustPlayingVolume(OpenALPlayingAudio* audio)
{
	Real desiredVolume = audio->m_audioEventRTS->getVolume() * audio->m_audioEventRTS->getVolumeShift();
	if (audio->m_type == PAT_Sample) {
		alSourcef(audio->source, AL_GAIN, m_soundVolume * desiredVolume);
	}
	else if (audio->m_type == PAT_3DSample) {
		alSourcef(audio->source, AL_GAIN, m_sound3DVolume * desiredVolume);
	}
	else if (audio->m_type == PAT_Stream) {
		if (audio->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			alSourcef(audio->source, AL_GAIN, m_sound3DVolume * m_musicVolume * desiredVolume);
		}
		else {
			alSourcef(audio->source, AL_GAIN, m_sound3DVolume * m_speechVolume * desiredVolume);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::processPlayingList(void)
{
	// There are two types of processing we have to do here. 
	// 1. Move the item to the stopped list if it has become stopped.
	// 2. Update the position of the audio if it is positional
	std::list<OpenALPlayingAudio*>::iterator it;
	OpenALPlayingAudio* playing;

	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); /* empty */) {
		playing = (*it);
		if (!playing)
		{
			it = m_playingSounds.erase(it);
			continue;
		}

		if (playing->m_status == PS_Stopped)
		{
			//m_stoppedAudio.push_back(playing);
			releasePlayingAudio(playing);
			it = m_playingSounds.erase(it);
		}
		else
		{
			if (m_volumeHasChanged)
			{
				adjustPlayingVolume(playing);
			}
			++it;
		}
	}

	for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); )
	{
		playing = (*it);
		if (!playing)
		{
			it = m_playing3DSounds.erase(it);
			continue;
		}

		if (playing->m_status == PS_Stopped)
		{
			//m_stoppedAudio.push_back(playing);			
			releasePlayingAudio(playing);
			it = m_playing3DSounds.erase(it);
		}
		else
		{
			if (m_volumeHasChanged)
			{
				adjustPlayingVolume(playing);
			}

			const Coord3D* pos = getCurrentPositionFromEvent(playing->m_audioEventRTS);
			if (pos)
			{
				if (playing->m_audioEventRTS->isDead())
				{
					stopAudioEvent(playing->m_audioEventRTS->getPlayingHandle());
					it++;
					continue;
				}
				else
				{
					Real volForConsideration = getEffectiveVolume(playing->m_audioEventRTS);
					volForConsideration /= (m_sound3DVolume > 0.0f ? m_soundVolume : 1.0f);
					Bool playAnyways = BitTestEA(playing->m_audioEventRTS->getAudioEventInfo()->m_type, ST_GLOBAL) || playing->m_audioEventRTS->getAudioEventInfo()->m_priority == AP_CRITICAL;
					if (volForConsideration < m_audioSettings->m_minVolume && !playAnyways)
					{
						// don't want to get an additional callback for this sample
					//	AIL_register_3D_EOS_callback(playing->m_3DSample, NULL);
						//m_stoppedAudio.push_back(playing);
						releasePlayingAudio(playing);
						it = m_playing3DSounds.erase(it);
						continue;
					}
					else
					{
						alSource3f(playing->source, AL_POSITION, pos->x, pos->y, pos->z);
					}
				}
			}
			else
			{
			//	AIL_register_3D_EOS_callback(playing->m_3DSample, NULL);
				//m_stoppedAudio.push_back(playing);
				releasePlayingAudio(playing);
				it = m_playing3DSounds.erase(it);
				continue;
			}

			++it;
		}
	}

	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ) {
		playing = (*it);
		if (!playing)
		{
			it = m_playingStreams.erase(it);
			continue;
		}

		if (playing->m_status == PS_Stopped)
		{
			//m_stoppedAudio.push_back(playing);			
			releasePlayingAudio(playing);
			it = m_playingStreams.erase(it);
		}
		else
		{
			if (m_volumeHasChanged)
			{
				adjustPlayingVolume(playing);
			}

			++it;
		}
	}

	if (m_volumeHasChanged) {
		m_volumeHasChanged = false;
	}
}


float OpenALAudioManager::getEffectiveVolume(AudioEventRTS* event) const
{
	Real volume = 1.0f;
	volume *= (event->getVolume() * event->getVolumeShift());
	if (event->getAudioEventInfo()->m_soundType == AT_Music)
	{
		volume *= m_musicVolume;
	}
	else if (event->getAudioEventInfo()->m_soundType == AT_Streaming)
	{
		volume *= m_speechVolume;
	}
	else
	{
		if (event->isPositionalAudio())
		{
			volume *= m_sound3DVolume;
			Coord3D distance = m_listenerPosition;
			const Coord3D* pos = event->getCurrentPosition();
			if (pos)
			{
				distance.sub(pos);
				Real objMinDistance;
				Real objMaxDistance;

				if (event->getAudioEventInfo()->m_type & ST_GLOBAL)
				{
					objMinDistance = TheAudio->getAudioSettings()->m_globalMinRange;
					objMaxDistance = TheAudio->getAudioSettings()->m_globalMaxRange;
				}
				else
				{
					objMinDistance = event->getAudioEventInfo()->m_minDistance;
					objMaxDistance = event->getAudioEventInfo()->m_maxDistance;
				}

				Real objDistance = distance.length();
				if (objDistance > objMinDistance)
				{
					volume *= 1 / (objDistance / objMinDistance);
				}
				if (objDistance >= objMaxDistance)
				{
					volume = 0.0f;
				}
				//else if( objDistance > objMinDistance )
				//{
				//	volume *= 1.0f - (objDistance - objMinDistance) / (objMaxDistance - objMinDistance);
				//}
			}
		}
		else
		{
			volume *= m_soundVolume;
		}
	}

	return volume;
}
