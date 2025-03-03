#pragma once

#include <string>
#include <AL/al.h>

/**
 * \brief A utility class to load WAV or MP3 files from your custom FileSystem into OpenAL buffers.
 */
class OpenALAudioLoader
{
public:
    static ALuint loadFromFile(const std::string& filename);

private:
    /// Decode WAV data from an in-memory buffer, return OpenAL buffer ID.
    static ALuint decodeWav(const char* data, size_t dataSize);

    /// Decode MP3 data from an in-memory buffer, return OpenAL buffer ID.
    static ALuint decodeMp3(const char* data, size_t dataSize);

    /// A helper to check if filename ends with a particular extension (case-insensitive).
    static bool hasExtension(const std::string& filename, const char* ext);

    /// Internal cache: file path -> OpenAL buffer
    static bool s_initializedCache;
    static std::unordered_map<std::string, ALuint> s_bufferCache;
};