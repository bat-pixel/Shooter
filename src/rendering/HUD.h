#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class HUD {
public:
    HUD(TTF_Font* font, SDL_Texture* lifeIcon);

    void render(SDL_Renderer* renderer,
                int score, int lives, int stage, int shieldLevel,
                int loopsRemaining, int highScore);

private:
    void renderText(SDL_Renderer* r, const std::string& text,
                    float x, float y, SDL_Color col);

    TTF_Font*    m_font;
    SDL_Texture* m_lifeIcon;
};
