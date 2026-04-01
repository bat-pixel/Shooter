#pragma once
#include <SDL3_mixer/SDL_mixer.h>
#include <string>
#include <unordered_map>

class AudioManager {
public:
    static AudioManager& get();

    void init(const std::string& basePath);
    void shutdown();

    // loops: -1 = infinite, 0 = once
    void playMusic(const std::string& relativePath, int loops = -1);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();

    void playSound(const std::string& relativePath);

    void setMusicGain(float gain);   // 0.0–1.0
    void setSFXGain(float gain);

private:
    AudioManager() = default;
    ~AudioManager() = default;
    AudioManager(const AudioManager&) = delete;

    MIX_Audio* loadAudio(const std::string& rel);

    std::string  m_base;
    MIX_Mixer*   m_mixer      = nullptr;
    MIX_Track*   m_musicTrack = nullptr;
    float        m_sfxGain    = 1.0f;

    std::unordered_map<std::string, MIX_Audio*> m_audioCache;
};
