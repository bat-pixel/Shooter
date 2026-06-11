#include "HUD.h"
#include "../Constants.h"
#include <string>

HUD::HUD(TTF_Font* font, SDL_Texture* lifeIcon)
    : m_font(font), m_lifeIcon(lifeIcon) {}

void HUD::render(SDL_Renderer* renderer,
                 int score, int lives, int stage, int /*shieldLevel*/,
                 int loopsRemaining, int highScore,
                 const std::string& campaign) {
    const float PAD = 8.f;

    // ── Top row ──────────────────────────────────────────────────────────
    // Score (top-left)
    renderText(renderer, std::to_string(score), PAD, PAD, {255, 255, 100, 255});

    // Hi-score (top-centre)
    std::string hs = "HI " + std::to_string(highScore);
    renderText(renderer, hs, LOGICAL_W * 0.5f - 44.f, PAD, {200, 200, 200, 255});

    // Life icons — plane sprites (top-right)
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

    // ── Bottom row ───────────────────────────────────────────────────────
    // Loop charge indicators — small cyan bars (not plane sprites)
    for (int i = 0; i < loopsRemaining; ++i) {
        SDL_FRect bar = {PAD + i * 9.f, LOGICAL_H - PAD - 10.f, 7.f, 10.f};
        SDL_SetRenderDrawColor(renderer, 80, 210, 255, 220);
        SDL_RenderFillRect(renderer, &bar);
        SDL_SetRenderDrawColor(renderer, 140, 240, 255, 255);
        SDL_RenderRect(renderer, &bar);
    }
    if (loopsRemaining > 0)
        renderText(renderer, "LOOP", PAD, LOGICAL_H - PAD - 24.f, {80, 210, 255, 200});

    // Campaign / area name (bottom-centre)
    if (!campaign.empty()) {
        float cx = LOGICAL_W * 0.5f - (float)(campaign.size() * 5.f);
        renderText(renderer, campaign, cx, LOGICAL_H - PAD - 14.f, {180, 220, 255, 200});
    }

    // Stage number (bottom-right)
    std::string stageTxt = "STAGE " + std::to_string(stage);
    renderText(renderer, stageTxt, LOGICAL_W - 100.f, LOGICAL_H - PAD - 14.f, {220, 220, 220, 220});
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
