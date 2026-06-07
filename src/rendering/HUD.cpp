#include "HUD.h"
#include "../Constants.h"
#include <string>

HUD::HUD(TTF_Font* font, SDL_Texture* lifeIcon,
         std::array<SDL_Texture*, 10> numerals)
    : m_font(font), m_lifeIcon(lifeIcon), m_numerals(numerals) {}

void HUD::render(SDL_Renderer* renderer,
                 int score, int lives, int stage, int /*shieldLevel*/,
                 int loopsRemaining, int highScore) {
    const float PAD   = 8.f;
    const float NSCALE = 0.6f;

    // Score — top left
    renderNumber(renderer, score, PAD, PAD, NSCALE);

    // High score — top center
    {
        std::string label = "HI ";
        float lx = LOGICAL_W * 0.5f - 50.f;
        renderText(renderer, label, lx, PAD, {200, 200, 200, 255});
        float tw = 0;
        if (m_font) {
            SDL_Surface* s = TTF_RenderText_Blended(m_font, label.c_str(), label.size(), {255,255,255,255});
            if (s) { tw = (float)s->w; SDL_DestroySurface(s); }
        }
        renderNumber(renderer, highScore, lx + tw + 4, PAD, NSCALE);
    }

    // Lives — top right
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

    // Loop counter — bottom left (small life-icon-sized slots)
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

    // Stage — bottom right
    if (m_font) {
        std::string stageTxt = "STAGE " + std::to_string(stage);
        renderText(renderer, stageTxt,
                   LOGICAL_W - 110.f, LOGICAL_H - 24.f,
                   {220, 220, 220, 255});
    }
}

void HUD::renderNumber(SDL_Renderer* r, int value, float x, float y, float scale) {
    std::string s = std::to_string(value);
    float nx = x;
    for (char c : s) {
        int digit = c - '0';
        if (digit < 0 || digit > 9 || !m_numerals[digit]) continue;
        float nw, nh;
        SDL_GetTextureSize(m_numerals[digit], &nw, &nh);
        SDL_FRect dst = {nx, y, nw * scale, nh * scale};
        SDL_RenderTexture(r, m_numerals[digit], nullptr, &dst);
        nx += dst.w + 2;
    }
}

void HUD::renderText(SDL_Renderer* r, const std::string& text,
                     float x, float y, SDL_Color col) {
    if (!m_font) return;
    SDL_Surface* surf = TTF_RenderText_Blended(m_font, text.c_str(),
                                               text.size(), col);
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
