#include "HUD.h"
#include "../Constants.h"
#include <string>

HUD::HUD(TTF_Font* font, SDL_Texture* lifeIcon,
         std::array<SDL_Texture*, 10> numerals)
    : m_font(font), m_lifeIcon(lifeIcon), m_numerals(numerals) {}

void HUD::render(SDL_Renderer* renderer,
                 int score, int lives, int level, int /*shieldLevel*/) {
    const float PAD = 8.f;

    // Score — top left using sprite numerals
    std::string s = std::to_string(score);
    float nx = PAD;
    for (char c : s) {
        int digit = c - '0';
        if (digit >= 0 && digit <= 9 && m_numerals[digit]) {
            float nw, nh;
            SDL_GetTextureSize(m_numerals[digit], &nw, &nh);
            SDL_FRect dst = {nx, PAD, nw * 0.6f, nh * 0.6f};
            SDL_RenderTexture(renderer, m_numerals[digit], nullptr, &dst);
            nx += dst.w + 2;
        }
    }

    // Lives — top right using life icons
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

    // Level — top center using font fallback
    if (m_font) {
        std::string lvlTxt = "LVL " + std::to_string(level);
        renderText(renderer, lvlTxt,
                   LOGICAL_W * 0.5f - 20, PAD,
                   {220, 220, 220, 255});
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
