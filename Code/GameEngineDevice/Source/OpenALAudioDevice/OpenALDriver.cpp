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

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::openDevice(void)
{
	device = alcOpenDevice(nullptr); // Open default device
	if (!device)
	{
		DEBUG_CRASH(("Failed to open OpenAL device."));
		return;
	}
	context = alcCreateContext(device, nullptr);
	if (!context)
	{
		DEBUG_CRASH(("Failed to create OpenAL context."));
		alcCloseDevice(device);
		device = nullptr;
		return;
	}
	alcMakeContextCurrent(context);
	m_digitalHandle = device;
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::releasePlayingAudio(OpenALPlayingAudio* release)
{
	// Stop Audio Here
	if (release)
	{
		if (release->source)
			alDeleteSources(1, &release->source);
		//if (release->buffer)
			//alDeleteBuffers(1, &release->buffer);
		delete release;
	}
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::closeDevice(void)
{
	alcMakeContextCurrent(nullptr);
	if (context)
	{
		alcDestroyContext(context);
		context = nullptr;
	}
	if (device)
	{
		alcCloseDevice(device);
		device = nullptr;
	}
	m_digitalHandle = nullptr;
}
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::update()
{
    AudioManager::update();

	processRequestList();
	processPlayingList();
	processFadingList();
	processStoppedList();
	setDeviceListenerPosition();
}

//-------------------------------------------------------------------------------------------------
Bool OpenALAudioManager::hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const
{
	std::list<OpenALPlayingAudio*>::const_iterator it;
	OpenALPlayingAudio* playing;
	for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			if (playing->m_audioEventRTS->getEventName() == trackName) {
				//if (INFINITE_LOOP_COUNT - AIL_stream_loop_count(playing->m_stream) >= numberOfTimes) {
				//	return TRUE;
				//}
				// IMPLEMENT TESTING IF MUSIC IS PLAYING OR NOT 
			}
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::playSample(AudioEventRTS* event, OpenALPlayingAudio* audio, bool isMusic)
{
	ALuint buffer = openFile(event);
	if (buffer == 0)
	{
		// Error loading file
		return;
	}
	audio->buffer = buffer;
	alGenSources(1, &audio->source);
	alSourcei(audio->source, AL_BUFFER, buffer);
	Real volume = event->getVolume();
	alSourcef(audio->source, AL_GAIN, volume);
	if (isMusic)
	{
		alSourcei(audio->source, AL_LOOPING, AL_TRUE);
	}
	alSourcePlay(audio->source);
}

//-------------------------------------------------------------------------------------------------
bool OpenALAudioManager::playSample3D(AudioEventRTS* event, OpenALPlayingAudio* audio)
{
	const Coord3D* pos = getCurrentPositionFromEvent(event);
	if (pos)
	{
		ALuint buffer = openFile(event);
		if (buffer == 0)
			return false;
		audio->buffer = buffer;
		alGenSources(1, &audio->source);
		alSourcei(audio->source, AL_BUFFER, buffer);
		alSource3f(audio->source, AL_POSITION, pos->x, pos->y, pos->z);
		Real volume = event->getVolume();
		alSourcef(audio->source, AL_GAIN, volume);
		alSourcef(audio->source, AL_REFERENCE_DISTANCE, event->getAudioEventInfo()->m_minDistance);
		alSourcef(audio->source, AL_MAX_DISTANCE, event->getAudioEventInfo()->m_maxDistance);
		alSourcePlay(audio->source);
		return true;
	}
	return false;
}

void OpenALAudioManager::stopAudio(AudioAffect which)
{
	OpenALPlayingAudio* playing = nullptr;
	if (BitTestEA(which, AudioAffect_Sound))
	{
		for (auto it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it)
		{
			playing = *it;
			if (playing)
			{
				alSourceStop(playing->source);
				playing->m_status = PS_Stopped;
			}
		}
	}
	if (BitTestEA(which, AudioAffect_Sound3D))
	{
		for (auto it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it)
		{
			playing = *it;
			if (playing)
			{
				alSourceStop(playing->source);
				playing->m_status = PS_Stopped;
			}
		}
	}
	if (BitTestEA(which, AudioAffect_Speech | AudioAffect_Music))
	{
		for (auto it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it)
		{
			playing = *it;
			if (playing)
			{
				if (playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music)
				{
					if (!BitTestEA(which, AudioAffect_Music))
						continue;
				}
				else
				{
					if (!BitTestEA(which, AudioAffect_Speech))
						continue;
				}
				alSourceStop(playing->source);
				playing->m_status = PS_Stopped;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::pauseAudio(AudioAffect which)
{
	OpenALPlayingAudio* playing = nullptr;
	if (BitTestEA(which, AudioAffect_Sound))
	{
		for (auto it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it)
		{
			playing = *it;
			if (playing)
				alSourcePause(playing->source);
		}
	}
	if (BitTestEA(which, AudioAffect_Sound3D))
	{
		for (auto it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it)
		{
			playing = *it;
			if (playing)
				alSourcePause(playing->source);
		}
	}
	if (BitTestEA(which, AudioAffect_Speech | AudioAffect_Music))
	{
		for (auto it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it)
		{
			playing = *it;
			if (playing)
			{
				if (playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music)
				{
					if (!BitTestEA(which, AudioAffect_Music))
						continue;
				}
				else
				{
					if (!BitTestEA(which, AudioAffect_Speech))
						continue;
				}
				alSourcePause(playing->source);
			}
		}
	}
	// Remove pending PLAY requests while pausing.
	for (auto ait = m_audioRequests.begin(); ait != m_audioRequests.end(); )
	{
		AudioRequest* req = *ait;
		if (req && req->m_request == AR_Play)
		{
			req->deleteInstance();
			ait = m_audioRequests.erase(ait);
		}
		else
		{
			++ait;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::resumeAudio(AudioAffect which)
{
	OpenALPlayingAudio* playing = nullptr;
	if (BitTestEA(which, AudioAffect_Sound))
	{
		for (auto it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it)
		{
			playing = *it;
			if (playing)
				alSourcePlay(playing->source);
		}
	}
	if (BitTestEA(which, AudioAffect_Sound3D))
	{
		for (auto it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ++it)
		{
			playing = *it;
			if (playing)
				alSourcePlay(playing->source);
		}
	}
	if (BitTestEA(which, AudioAffect_Speech | AudioAffect_Music))
	{
		for (auto it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it)
		{
			playing = *it;
			if (playing)
			{
				if (playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music)
				{
					if (!BitTestEA(which, AudioAffect_Music))
						continue;
				}
				else
				{
					if (!BitTestEA(which, AudioAffect_Speech))
						continue;
				}
				alSourcePlay(playing->source);
			}
		}
	}
}

void OpenALAudioManager::processStoppedList(void)
{
	auto checkAndRelease = [&](std::list<OpenALPlayingAudio*>& audioList)
		{
			for (auto it = audioList.begin(); it != audioList.end(); )
			{
				OpenALPlayingAudio* audio = *it;
				if (audio && audio->source != 0 && alIsSource(audio->source))
				{
					ALint state = 0;
					alGetSourcei(audio->source, AL_SOURCE_STATE, &state);

					// If the source is done playing or was never started properly
					if (state == AL_STOPPED || state == AL_INITIAL)
					{
						// Release and erase from the list
						releasePlayingAudio(audio);

						it = audioList.erase(it);
						continue; // Avoid incrementing 'it' again
					}
				}
				++it;
			}
		};

	// Check the three different lists
	checkAndRelease(m_playingSounds);
	checkAndRelease(m_playing3DSounds);
	checkAndRelease(m_playingStreams);
}

void OpenALAudioManager::setDeviceListenerPosition(void)
{
	Real x = m_listenerPosition.x;
	Real y = m_listenerPosition.y;
	Real z = m_listenerPosition.z;

	alListener3f(AL_POSITION, x, y, z);

	ALfloat listenerOri[6] = {
		0.612f, -0.5f,  0.612f,  // Forward vector: direction camera is facing
		0.354f,  0.866f, 0.354f   // Up vector: defines "up" on the screen
	};

	alListenerfv(AL_ORIENTATION, listenerOri);
}


void OpenALAudioManager::processFadingList(void)
{

}