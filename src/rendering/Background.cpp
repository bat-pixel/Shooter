#include "Background.h"
#include "../Constants.h"
#include <cmath>

Background::Background(std::vector<SDL_Texture*> textures, float scrollSpeed)
    : m_textures(std::move(textures)), m_speed(scrollSpeed)
{
    // Determine rendered segment height from the first valid texture.
    // Each texture is scaled to fill LOGICAL_W wide; height scaled proportionally.
    for (auto* t : m_textures) {
        if (t) {
            float tw, th;
            SDL_GetTextureSize(t, &tw, &th);
            if (tw > 0) { m_segH = th * ((float)LOGICAL_W / tw); break; }
        }
    }
}

void Background::update(float dt) {
    // Offset grows → segments shift upward → content appears to scroll downward
    m_offsetY -= m_speed * dt;
    float totalH = m_segH * (float)m_textures.size();
    if (totalH > 0) {
        // Keep offset in [-totalH, 0] range
        m_offsetY = fmodf(m_offsetY, totalH);
        if (m_offsetY > 0) m_offsetY -= totalH;
    }
}

void Background::render(SDL_Renderer* renderer) {
    if (m_textures.empty() || m_segH <= 0) return;
    int n = (int)m_textures.size();

    // Find first segment that could be on-screen, then draw forward until off bottom
    int startK = (int)floorf(m_offsetY / m_segH) - 1;
    for (int k = startK; k <= startK + (int)ceilf((float)LOGICAL_H / m_segH) + 2; ++k) {
        float y = k * m_segH - m_offsetY;
        if (y > (float)LOGICAL_H) break;
        if (y + m_segH < 0)       continue;

        int idx = ((k % n) + n) % n;
        if (!m_textures[idx]) continue;

        SDL_FRect dst = {0.f, y, (float)LOGICAL_W, m_segH};
        SDL_RenderTexture(renderer, m_textures[idx], nullptr, &dst);
    }
}
