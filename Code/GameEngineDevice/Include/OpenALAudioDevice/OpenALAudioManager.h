#pragma once

#include "Common/AsciiString.h"
#include "Common/GameAudio.h"

#include <AL/al.h>
#include <AL/alc.h>

#undef stopAudioEvent

#define NUM_POOLED_SOURCES2D 32
#define NUM_POOLED_SOURCES3D 128

enum OpenALPlayingAudioType
{
    PAT_Sample,
    PAT_3DSample,
    PAT_Stream,
    PAT_INVALID
};

enum OpenALPlayingStatus
{
    PS_Playing,
    PS_Stopped,
    PS_Paused
};

enum OpenALPlayingWhich
{
    PW_Attack,
    PW_Sound,
    PW_Decay,
    PW_INVALID
};

struct OpenALPlayingAudio
{
    ALuint source;
    ALuint buffer;

    ALuint poolIndex;

    OpenALPlayingAudioType m_type;
    OpenALPlayingStatus m_status;
    AudioEventRTS* m_audioEventRTS;
    Bool m_requestStop;
    Bool m_cleanupAudioEventRTS;
    Int m_framesFaded;

    OpenALPlayingAudio() :
        m_type(PAT_INVALID),
        m_audioEventRTS(NULL),
        m_requestStop(false),
        m_cleanupAudioEventRTS(true),
        m_framesFaded(0)
    {
    }
};

class OpenALAudioManager : public AudioManager
{
public:
#if defined(_DEBUG) || defined(_INTERNAL)
    virtual void audioDebugDisplay(DebugDisplayInterface* dd, void* data, FILE* fp = NULL);
    virtual AudioHandle addAudioEvent(const AudioEventRTS* eventToAdd);
#endif

    // from AudioDevice
    virtual void init();
    virtual void postProcessLoad();
    virtual void reset();
    virtual void update();

    OpenALAudioManager();
    virtual ~OpenALAudioManager();

    virtual void nextMusicTrack(void);
    virtual void prevMusicTrack(void);
    virtual Bool isMusicPlaying(void) const;
    virtual Bool hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const;
    virtual AsciiString getMusicTrackName(void) const;

    virtual void openDevice(void);
    virtual void closeDevice(void);
    virtual void* getDevice();

    virtual void stopAudio(AudioAffect which);
    virtual void pauseAudio(AudioAffect which);
    virtual void resumeAudio(AudioAffect which);
    virtual void pauseAmbient(Bool shouldPause);

    virtual void killAudioEventImmediately(AudioHandle audioEvent);

    virtual void stopAllAmbientsBy(Object* objID);
    virtual void stopAllAmbientsBy(Drawable* drawID);

    ///< NOTE: Do not use this for game logic purposes!
    virtual Bool isCurrentlyPlaying(AudioHandle handle);

    virtual void notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags);

    virtual UnsignedInt getProviderCount(void) const;
    virtual AsciiString getProviderName(UnsignedInt providerNum) const;
    virtual UnsignedInt getProviderIndex(AsciiString providerName) const;
    virtual void selectProvider(UnsignedInt providerNdx);
    virtual void unselectProvider(void);
    virtual UnsignedInt getSelectedProvider(void) const;
    virtual void setSpeakerType(UnsignedInt speakerType);
    virtual UnsignedInt getSpeakerType(void);

    virtual void* getHandleForBink(void);
    virtual void releaseHandleForBink(void);

    virtual void friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay);

    virtual UnsignedInt getNum2DSamples(void) const;
    virtual UnsignedInt getNum3DSamples(void) const;
    virtual UnsignedInt getNumStreams(void) const;

    virtual Bool doesViolateLimit(AudioEventRTS* event) const;
    virtual Bool isPlayingLowerPriority(AudioEventRTS* event) const;
    virtual Bool isPlayingAlready(AudioEventRTS* event) const;
    virtual Bool isObjectPlayingVoice(UnsignedInt objID) const;
    Bool killLowestPrioritySoundImmediately(AudioEventRTS* event);
    AudioEventRTS* findLowestPrioritySound(AudioEventRTS* event);

    virtual void adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume);

    virtual void removePlayingAudio(AsciiString eventName);
    virtual void removeAllDisabledAudio();

    virtual void processRequestList(void);
    virtual void processPlayingList(void);
    virtual void processFadingList(void);
    virtual void processStoppedList(void);

    Bool shouldProcessRequestThisFrame(AudioRequest* req) const;
    void adjustRequest(AudioRequest* req);
    Bool checkForSample(AudioRequest* req);

    virtual void setHardwareAccelerated(Bool accel);
    virtual void setSpeakerSurround(Bool surround);

    virtual void setPreferredProvider(AsciiString provider) { m_pref3DProvider = provider; }
    virtual void setPreferredSpeaker(AsciiString speakerType) { m_prefSpeaker = speakerType; }

    virtual Real getFileLengthMS(AsciiString strToLoad) const;

    virtual void closeAnySamplesUsingFile(const void* fileToClose);
protected:
    // 3-D functions
    virtual void setDeviceListenerPosition(void);
protected:
    void processRequest(AudioRequest* req);
    void playAudioEvent(AudioEventRTS* event);
    void stopAudioEvent(AudioHandle handle);
    void pauseAudioEvent(AudioHandle handle);

    OpenALPlayingAudio* allocatePlayingAudio(void);

    const Coord3D* getCurrentPositionFromEvent(AudioEventRTS* event);

    void playSample(AudioEventRTS* event, OpenALPlayingAudio *audio, bool isMusic = false);
    bool playSample3D(AudioEventRTS* event, OpenALPlayingAudio* audio);
    ALuint openFile(AudioEventRTS* eventToOpenFrom);
    float getEffectiveVolume(AudioEventRTS* event) const;

    void adjustPlayingVolume(OpenALPlayingAudio* audio);
    void stopAllSpeech(void);
    void stopAllAudioImmediately(void);
    void releasePlayingAudio(OpenALPlayingAudio* release);
    void recycleSource(ALuint poolIndex, bool is3D);
    ALuint getFreeSource(ALuint&poolIndex, bool is3D);

    void* m_digitalHandle;

    ALuint m_musicSource;

    AsciiString m_pref3DProvider;
    AsciiString m_prefSpeaker;

    ALCdevice* device;
    ALCcontext* context;

    UnsignedInt m_selectedSpeakerType;

    // Currently Playing stuff. Useful if we have to preempt it. 
    // This should rarely if ever happen, as we mirror this in Sounds, and attempt to 
    // keep preemption from taking place here.
    std::list<OpenALPlayingAudio*> m_playingSounds;
    std::list<OpenALPlayingAudio*> m_playing3DSounds;
    std::list<OpenALPlayingAudio*> m_playingStreams;

    // Currently fading stuff. At this point, we just want to let it finish fading, when it is
    // done it should be added to the completed list, then "freed" and the counts should be updated
    // on the next update
    std::list<OpenALPlayingAudio*> m_fadingAudio;

    // Stuff that is done playing (either because it has finished or because it was killed)
    // This stuff should be cleaned up during the next update cycle. This includes updating counts
    // in the sound engine
    std::list<OpenALPlayingAudio*> m_stoppedAudio;

    private:
        ALuint m_sourcePool2D[NUM_POOLED_SOURCES2D];
        bool m_sourceInUse2D[NUM_POOLED_SOURCES2D];

        ALuint m_sourcePool3D[NUM_POOLED_SOURCES3D];
        bool m_sourceInUse3D[NUM_POOLED_SOURCES3D];

        std::vector<ALuint> m_buffers;
};
