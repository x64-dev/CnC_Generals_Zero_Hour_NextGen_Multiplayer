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

#define NUM_POOLED_SOURCES 32

ALuint m_sourcePool2D[NUM_POOLED_SOURCES];
bool m_sourceInUse2D[NUM_POOLED_SOURCES];

ALuint m_sourcePool3D[NUM_POOLED_SOURCES];
bool m_sourceInUse3D[NUM_POOLED_SOURCES];

//-----------------------------------------------------------------------------
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

    alGenSources(1, &m_musicSource);  // This source is used ONLY for music

    // Create the pool of SFX sources
    alGenSources(NUM_POOLED_SOURCES, m_sourcePool2D);
    alGenSources(NUM_POOLED_SOURCES, m_sourcePool3D);

    // Mark them all as free initially
    for (int i = 0; i < NUM_POOLED_SOURCES; ++i)
    {
        m_sourceInUse2D[i] = false;
        m_sourceInUse3D[i] = false;
    }
}

//-----------------------------------------------------------------------------
// Helper to get a free source from the pool
ALuint OpenALAudioManager::getFreeSource(ALuint &poolIndex, bool is3D)
{
    if (is3D)
    {
		for (int i = 0; i < NUM_POOLED_SOURCES; ++i)
		{
			if (!m_sourceInUse3D[i])
			{
				poolIndex = i;
				m_sourceInUse3D[i] = true;

				// Reset the source so it is ready for reuse
				alSourceStop(m_sourcePool3D[i]);
				alSourcei(m_sourcePool3D[i], AL_BUFFER, 0);

				return m_sourcePool3D[i];
			}
		}
    }
    else
    {
		for (int i = 0; i < NUM_POOLED_SOURCES; ++i)
		{
			if (!m_sourceInUse2D[i])
			{
				poolIndex = i;
				m_sourceInUse2D[i] = true;

				// Reset the source so it is ready for reuse
				alSourceStop(m_sourcePool2D[i]);
				alSourcei(m_sourcePool2D[i], AL_BUFFER, 0);

				return m_sourcePool2D[i];
			}
		}
    }
   
    // No free source available!
    return -1;
}

//-----------------------------------------------------------------------------
// Helper to recycle a source back to the pool
void OpenALAudioManager::recycleSource(ALuint poolIndex, bool is3D)
{
    if (is3D)
    {
		alSourceStop(m_sourcePool3D[poolIndex]);
		alSourcei(m_sourcePool3D[poolIndex], AL_BUFFER, 0);
		m_sourceInUse3D[poolIndex] = false;
    }
    else
    {
		alSourceStop(m_sourcePool2D[poolIndex]);
		alSourcei(m_sourcePool2D[poolIndex], AL_BUFFER, 0);
		m_sourceInUse2D[poolIndex] = false;
    }
}

//-----------------------------------------------------------------------------
void OpenALAudioManager::releasePlayingAudio(OpenALPlayingAudio* release)
{
    if (release->m_audioEventRTS && release->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_SoundEffect) {
        if (release->m_type == PAT_Sample) {
            if (release->source != -1) {
                m_sound->notifyOf2DSampleCompletion();
            }
        }
        else {
            if (release->source != -1) {
                m_sound->notifyOf3DSampleCompletion();
            }
        }
    }

    // Instead of deleting the source, recycle it
    if (release)
    {
        if (release->source != 0)
        {
            // If it's the dedicated music source, don't recycle into the SFX pool
            if (release->source == m_musicSource)
            {
                // Just stop the music source. We keep it forever.
                alSourceStop(m_musicSource);
                alSourcei(m_musicSource, AL_BUFFER, 0);
            }
            else
            {
                // SFX source => recycle it in the pool
                if (release->m_type == PAT_3DSample) {
                    recycleSource(release->poolIndex, true);
                }
                else {
                    recycleSource(release->poolIndex, false);
                }
            }
        }
        // Buffers are typically managed externally or can remain cached
        // if you are reusing the same audio data. Adjust as needed.
        // if (release->buffer) alDeleteBuffers(1, &release->buffer);

        delete release;
    }
}

//-----------------------------------------------------------------------------
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
        // Delete all SFX sources in the pool
        alDeleteSources(NUM_POOLED_SOURCES, m_sourcePool2D);
        alDeleteSources(NUM_POOLED_SOURCES, m_sourcePool3D);

        // Delete the dedicated music source
        if (m_musicSource)
            alDeleteSources(1, &m_musicSource);

        alcCloseDevice(device);
        device = nullptr;
    }
    m_digitalHandle = nullptr;
}

//-----------------------------------------------------------------------------
// Modified so that music always uses the dedicated music source
// and SFX uses a pooled source
void OpenALAudioManager::playSample(AudioEventRTS* event, OpenALPlayingAudio* audio, bool isMusic)
{
    ALuint buffer = openFile(event);
    if (buffer == 0)
    {
        // Error loading file
        return;
    }
    audio->buffer = buffer;

    ALuint source = 0;

    if (isMusic)
    {
        // Always use the dedicated music source
        source = m_musicSource;
    }
    else
    {
        // Get a source from the pool
        source = getFreeSource(audio->poolIndex, false);
        if (source == -1)
        {
            killLowestPrioritySoundImmediately(event);
            source = getFreeSource(audio->poolIndex, false);

            if (source == -1)
            {
                return;
            }
        }
    }
    audio->m_type = PAT_Sample;
    audio->source = source;
    alSourcei(source, AL_BUFFER, buffer);
    alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    Real volume = event->getVolume();
    alSourcef(source, AL_GAIN, volume);

    // If this is music, you might want to loop it
    if (isMusic)
    {
        alSourcei(source, AL_LOOPING, AL_TRUE);
    }

    alSourcePlay(source);
}

//-----------------------------------------------------------------------------
bool OpenALAudioManager::playSample3D(AudioEventRTS* event, OpenALPlayingAudio* audio)
{
    const Coord3D* pos = getCurrentPositionFromEvent(event);
    if (!pos)
        return false;

    ALuint buffer = openFile(event);
    if (buffer == 0)
        return false;

    audio->buffer = buffer;

    // Get a 3D source from the pool
    ALuint source = getFreeSource(audio->poolIndex, true);
    if (source == -1)
    {
        killLowestPrioritySoundImmediately(event);
        source = getFreeSource(audio->poolIndex, true);

        if (source == -1)
        {
            return false;
        }
    }

    audio->m_type = PAT_3DSample;
    audio->source = source;
    alSourcei(source, AL_BUFFER, buffer);

    // Set 3D position and volume
    alSource3f(source, AL_POSITION, pos->x, pos->y, pos->z);
    Real volume = event->getVolume();
    alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
    alSourcef(source, AL_GAIN, volume);
    alSourcef(source, AL_REFERENCE_DISTANCE, event->getAudioEventInfo()->m_minDistance);
    alSourcef(source, AL_MAX_DISTANCE, event->getAudioEventInfo()->m_maxDistance);

    alSourcePlay(source);
    return true;
}

//-----------------------------------------------------------------------------
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
                        // This will recycle or stop the source
                        releasePlayingAudio(audio);
                        it = audioList.erase(it);
                        continue;
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

//-----------------------------------------------------------------------------
void OpenALAudioManager::update()
{
    AudioManager::update();

    processRequestList();
    processPlayingList();
    processFadingList();
    processStoppedList();
    setDeviceListenerPosition();
}

//-----------------------------------------------------------------------------
void OpenALAudioManager::setDeviceListenerPosition(void)
{
    // Set the listener's orientation.
     // The first three elements are the "at" vector, the next three are the "up" vector.
    ALfloat listenerOri[6] = {
        m_listenerOrientation.x, m_listenerOrientation.y, m_listenerOrientation.z, // "at" vector
        0.0f, 0.0f, -1.0f  // "up" vector (as in the original code)
    };
    alListenerfv(AL_ORIENTATION, listenerOri);

    // Set the listener's position.
    ALfloat listenerPos[3] = {
        m_listenerPosition.x,
        m_listenerPosition.y,
        m_listenerPosition.z
    };
    alListenerfv(AL_POSITION, listenerPos);
}

//-----------------------------------------------------------------------------
void OpenALAudioManager::processFadingList(void)
{
    // Fading logic (unchanged)...
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