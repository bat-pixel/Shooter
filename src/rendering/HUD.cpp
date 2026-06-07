#include "HUD.h"
#include "../Constants.h"
#include <string>

HUD::HUD(TTF_Font* font, SDL_Texture* lifeIcon)
    : m_font(font), m_lifeIcon(lifeIcon) {}

void HUD::render(SDL_Renderer* renderer,
                 int score, int lives, int stage, int /*shieldLevel*/,
                 int loopsRemaining, int highScore) {
    const float PAD = 8.f;

    renderText(renderer, std::to_string(score), PAD, PAD, {255, 255, 100, 255});

    std::string hs = "HI " + std::to_string(highScore);
    renderText(renderer, hs, LOGICAL_W * 0.5f - 44.f, PAD, {200, 200, 200, 255});

    if (m_lifeIcon) {
        float iw, ih;
        SDL_GetTextureSize(m_lifeIcon, &iw, &ih);
        iw *= 0.5f; ih *= 0.5f;
        for (int i = 0; i < lives; ++i) {
            float lx = LOGICAL_W - PAD - (lives - i) * (iw + 4);
            SDL_FRect dst = {lx, PAD, iw, ih};
            SDL_RenderTexture(renderer, m_lifeIcon, nullptr, &dst);
        }
    }

    if (m_lifeIcon) {
        float iw, ih;
        SDL_GetTextureSize(m_lifeIcon, &iw, &ih);
        iw *= 0.4f; ih *= 0.4f;
        for (int i = 0; i < loopsRemaining; ++i) {
            SDL_FRect dst = {PAD + i * (iw + 3), LOGICAL_H - ih - PAD, iw, ih};
            SDL_SetTextureColorMod(m_lifeIcon, 100, 200, 255);
            SDL_RenderTexture(renderer, m_lifeIcon, nullptr, &dst);
            SDL_SetTextureColorMod(m_lifeIcon, 255, 255, 255);
        }
    }

    std::string stageTxt = "STAGE " + std::to_string(stage);
    renderText(renderer, stageTxt, LOGICAL_W - 110.f, LOGICAL_H - 24.f, {220, 220, 220, 255});
}

void HUD::renderText(SDL_Renderer* r, const std::string& text,
                     float x, float y, SDL_Color col) {
    if (!m_font) return;
    SDL_Surface* surf = TTF_RenderText_Blended(m_font, text.c_str(), text.size(), col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_DestroySurface(surf);
    if (!tex) return;
    float tw, th;
    SDL_GetTextureSize(tex, &tw, &th);
    SDL_FRect dst = {x, y, tw, th};
    SDL_RenderTexture(r, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}
