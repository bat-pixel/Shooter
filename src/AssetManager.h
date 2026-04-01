#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>

class AssetManager {
public:
    static AssetManager& get();

    void init(SDL_Renderer* renderer, const std::string& basePath);
    void shutdown();

    SDL_Texture* texture(const std::string& relativePath);
    TTF_Font*    font(const std::string& relativePath, int ptSize);

private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;

    SDL_Renderer* m_renderer = nullptr;
    std::string   m_base;
    std::unordered_map<std::string, SDL_Texture*> m_textures;
    std::unordered_map<std::string, TTF_Font*>    m_fonts;
};
