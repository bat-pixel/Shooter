#pragma once
#include <SDL3/SDL.h>

class Background {
public:
    Background(SDL_Texture* tex, float scrollSpeed = 60.0f);

    void update(float dt);
    void render(SDL_Renderer* renderer);

    void setSpeed(float s)         { m_speed = s; }
    void setTexture(SDL_Texture* tex);

private:
    SDL_Texture* m_tex;
    float        m_speed;
    float        m_offsetY = 0;
    float        m_texH    = 0;
    float        m_texW    = 0;
};
