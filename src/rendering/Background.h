#pragma once
#include <SDL3/SDL.h>
#include <vector>

class Background {
public:
    explicit Background(std::vector<SDL_Texture*> textures, float scrollSpeed = 80.0f);

    void update(float dt);
    void render(SDL_Renderer* renderer);
    void setSpeed(float s) { m_speed = s; }
    void setTextures(std::vector<SDL_Texture*> textures);

private:
    void computeTileSize();

    std::vector<SDL_Texture*> m_textures;
    float m_speed;
    float m_offsetY = 0;
    float m_segH    = 0;   // rendered height per tile row
    float m_tileW   = 0;   // rendered width per tile (tiles horizontally if < LOGICAL_W)
};
