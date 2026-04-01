#include "AssetManager.h"
#include <SDL3/SDL.h>

AssetManager& AssetManager::get() {
    static AssetManager instance;
    return instance;
}

void AssetManager::init(SDL_Renderer* renderer, const std::string& basePath) {
    m_renderer = renderer;
    m_base     = basePath;
}

void AssetManager::shutdown() {
    for (auto& [k, t] : m_textures) SDL_DestroyTexture(t);
    for (auto& [k, f] : m_fonts)    TTF_CloseFont(f);
    m_textures.clear();
    m_fonts.clear();
}

SDL_Texture* AssetManager::texture(const std::string& rel) {
    auto it = m_textures.find(rel);
    if (it != m_textures.end()) return it->second;

    std::string full = m_base + rel;
    SDL_Texture* t = IMG_LoadTexture(m_renderer, full.c_str());
    if (!t) {
        SDL_Log("AssetManager: failed to load texture '%s': %s",
                full.c_str(), SDL_GetError());
    }
    m_textures[rel] = t;
    return t;
}

TTF_Font* AssetManager::font(const std::string& rel, int ptSize) {
    std::string key = rel + ":" + std::to_string(ptSize);
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) return it->second;

    std::string full = m_base + rel;
    TTF_Font* f = TTF_OpenFont(full.c_str(), (float)ptSize);
    if (!f) {
        SDL_Log("AssetManager: failed to load font '%s': %s",
                full.c_str(), SDL_GetError());
    }
    m_fonts[key] = f;
    return f;
}
