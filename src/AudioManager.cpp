#include "AudioManager.h"
#include <SDL3/SDL.h>

AudioManager& AudioManager::get() {
    static AudioManager instance;
    return instance;
}

void AudioManager::init(const std::string& basePath) {
    m_base = basePath;

    if (!MIX_Init()) {
        SDL_Log("MIX_Init failed: %s", SDL_GetError());
        return;
    }

    m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!m_mixer) {
        SDL_Log("MIX_CreateMixerDevice failed: %s", SDL_GetError());
        return;
    }

    m_musicTrack = MIX_CreateTrack(m_mixer);
    if (!m_musicTrack) {
        SDL_Log("MIX_CreateTrack failed: %s", SDL_GetError());
    }
}

void AudioManager::shutdown() {
    for (auto& [k, a] : m_audioCache) MIX_DestroyAudio(a);
    m_audioCache.clear();

    if (m_mixer) {
        MIX_StopAllTracks(m_mixer, 0);
        MIX_DestroyMixer(m_mixer);
        m_mixer = nullptr;
    }
    m_musicTrack = nullptr;   // destroyed with mixer
    MIX_Quit();
}

MIX_Audio* AudioManager::loadAudio(const std::string& rel) {
    auto it = m_audioCache.find(rel);
    if (it != m_audioCache.end()) return it->second;

    std::string full = m_base + rel;
    MIX_Audio* audio = MIX_LoadAudio(m_mixer, full.c_str(), true);
    if (!audio) {
        SDL_Log("AudioManager: failed to load '%s': %s", full.c_str(), SDL_GetError());
    }
    m_audioCache[rel] = audio;
    return audio;
}

void AudioManager::playMusic(const std::string& rel, int loops) {
    if (!m_mixer || !m_musicTrack) return;
    MIX_Audio* audio = loadAudio(rel);
    if (!audio) return;

    MIX_StopTrack(m_musicTrack, 0);
    MIX_SetTrackAudio(m_musicTrack, audio);
    MIX_SetTrackLoops(m_musicTrack, loops);
    MIX_PlayTrack(m_musicTrack, 0);
}

void AudioManager::stopMusic() {
    if (m_musicTrack) MIX_StopTrack(m_musicTrack, 0);
}

void AudioManager::pauseMusic() {
    if (m_musicTrack) MIX_PauseTrack(m_musicTrack);
}

void AudioManager::resumeMusic() {
    if (m_musicTrack) MIX_ResumeTrack(m_musicTrack);
}

void AudioManager::playSound(const std::string& rel) {
    if (!m_mixer) return;
    MIX_Audio* audio = loadAudio(rel);
    if (!audio) return;
    MIX_PlayAudio(m_mixer, audio);
}

void AudioManager::setMusicGain(float gain) {
    if (m_musicTrack) MIX_SetTrackGain(m_musicTrack, gain);
}

void AudioManager::setSFXGain(float gain) {
    m_sfxGain = gain;
    if (m_mixer) MIX_SetMixerGain(m_mixer, gain);
}
