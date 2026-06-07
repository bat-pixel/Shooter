#pragma once
#include <SDL3/SDL.h>
#include <vector>

class Background {
public:
    explicit Background(std::vector<SDL_Texture*> textures, float scrollSpeed = 80.0f);

    void update(float dt);
    void render(SDL_Renderer* renderer);
    void setSpeed(float s) { m_speed = s; }

private:
    std::vector<SDL_Texture*> m_textures;
    float m_speed;
    float m_offsetY = 0;
    float m_segH    = 0; // rendered height of one texture (scaled to LOGICAL_W)
};
