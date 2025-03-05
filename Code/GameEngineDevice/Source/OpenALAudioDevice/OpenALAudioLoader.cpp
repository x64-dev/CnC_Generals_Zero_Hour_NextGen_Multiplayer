#include "Lib/Basetype.h"
#include "OpenALAudioDevice/OpenALAudioLoader.h"

#include <stdexcept>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <unordered_map>

#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION

#include "OpenALAudioDevice/dr_wav.h"
#include "OpenALAudioDevice/dr_mp3.h"
#include "Common/File.h"
#include "Common/FileSystemEA.h"

/** Static members */
bool OpenALAudioLoader::s_initializedCache = false;
std::unordered_map<std::string, ALuint> OpenALAudioLoader::s_bufferCache;

// ------------------------------------------------------
// Helper: check file extension (case-insensitive)
// ------------------------------------------------------
bool OpenALAudioLoader::hasExtension(const std::string& filename, const char* ext) {
    if (filename.size() < strlen(ext)) {
        return false;
    }
    std::string fileExt = filename.substr(filename.size() - strlen(ext));
    std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);

    std::string cmpExt(ext);
    std::transform(cmpExt.begin(), cmpExt.end(), cmpExt.begin(), ::tolower);

    return fileExt == cmpExt;
}

// ------------------------------------------------------
// Main load function that reads from your FileSystem
// ------------------------------------------------------
ALuint OpenALAudioLoader::loadFromFile(const std::string& filename) {
    if (!s_initializedCache) {
        s_bufferCache.clear();
        s_initializedCache = true;
    }

    auto it = s_bufferCache.find(filename);
    if (it != s_bufferCache.end()) {
        return it->second; // Already loaded, just return
    }

    File* file = TheFileSystem->openFile(filename.c_str());
    if (!file) {
        DEBUG_ASSERTLOG(filename.empty(), ("Missing Audio File: '%s'\n", filename.c_str()));
        return 0;
    }

    unsigned int fileSize = file->size();
    char* buffer = file->readEntireAndClose();
    if (!buffer || fileSize == 0) {
        delete[] buffer;
        return 0;
    }

    ALuint alBuffer = 0;
  //  try {
        if (hasExtension(filename, ".wav")) {
            alBuffer = decodeWav(buffer, fileSize);
        }
        else if (hasExtension(filename, ".mp3")) {
            alBuffer = decodeMp3(buffer, fileSize);
        }
        else {
            delete[] buffer;
            return 0;
        }
   // }
   // catch (const std::exception& e) {
   //     DEBUG_ASSERTLOG(false, ("AudioLoader error: %s\n", e.what()));
   //     alBuffer = 0;
   // }

    delete[] buffer;

    if (alBuffer != 0) {
        s_bufferCache[filename] = alBuffer;
    }

    return alBuffer;
}

// ------------------------------------------------------
// WAV decoding (using dr_wav)
// ------------------------------------------------------
ALuint OpenALAudioLoader::decodeWav(const char* data, size_t dataSize) {
    drwav wav;
    if (!drwav_init_memory(&wav, data, dataSize, nullptr)) {
        throw std::runtime_error("Failed to initialize WAV decoder");
    }

    std::vector<int16_t> pcmData(wav.totalPCMFrameCount * wav.channels);
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pcmData.data());
    drwav_uninit(&wav);

    ALenum format = (wav.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    ALuint bufferID;
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, format, pcmData.data(), pcmData.size() * sizeof(int16_t), wav.sampleRate);

    return bufferID;
}

// ------------------------------------------------------
// MP3 decoding (using dr_mp3)
// ------------------------------------------------------
ALuint OpenALAudioLoader::decodeMp3(const char* data, size_t dataSize) {
    drmp3 mp3;
    if (!drmp3_init_memory(&mp3, data, dataSize, nullptr)) {
        throw std::runtime_error("Failed to initialize MP3 decoder");
    }

    std::vector<int16_t> pcmData(drmp3_get_pcm_frame_count(&mp3) * mp3.channels);
    drmp3_read_pcm_frames_s16(&mp3, pcmData.size() / mp3.channels, pcmData.data());
    drmp3_uninit(&mp3);

    ALenum format = (mp3.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    ALuint bufferID;
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, format, pcmData.data(), pcmData.size() * sizeof(int16_t), mp3.sampleRate);

    return bufferID;
}
