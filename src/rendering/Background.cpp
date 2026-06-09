#include "Background.h"
#include "../Constants.h"
#include <cmath>
#include <algorithm>

static constexpr float BG_SCALE = 2.f;  // display tilesets at 2x native pixel size

Background::Background(std::vector<SDL_Texture*> textures, float scrollSpeed)
    : m_textures(std::move(textures)), m_speed(scrollSpeed)
{
    computeTileSize();
}

void Background::setTextures(std::vector<SDL_Texture*> textures) {
    m_textures = std::move(textures);
    computeTileSize();
}

void Background::computeTileSize() {
    m_segH  = 0;
    m_tileW = (float)LOGICAL_W;
    for (auto* t : m_textures) {
        if (!t) continue;
        float tw, th;
        SDL_GetTextureSize(t, &tw, &th);
        if (tw <= 0) continue;
        if (tw < (float)LOGICAL_W) {
            // Small tileset image — tile at BG_SCALE
            m_tileW = tw * BG_SCALE;
            m_segH  = th * BG_SCALE;
        } else {
            // Large image — stretch to fill full width
            m_tileW = (float)LOGICAL_W;
            m_segH  = th * ((float)LOGICAL_W / tw);
        }
        return;
    }
    m_segH  = (float)LOGICAL_H;
    m_tileW = (float)LOGICAL_W;
}

void Background::update(float dt) {
    m_offsetY -= m_speed * dt;
    float totalH = m_segH * (float)m_textures.size();
    if (totalH > 0) {
        m_offsetY = fmodf(m_offsetY, totalH);
        if (m_offsetY > 0) m_offsetY -= totalH;
    }
}

void Background::render(SDL_Renderer* renderer) {
    if (m_textures.empty() || m_segH <= 0 || m_tileW <= 0) return;
    int n = (int)m_textures.size();

    int startK = (int)floorf(m_offsetY / m_segH) - 1;
    for (int k = startK; k <= startK + (int)ceilf((float)LOGICAL_H / m_segH) + 2; ++k) {
        float y = k * m_segH - m_offsetY;
        if (y > (float)LOGICAL_H) break;
        if (y + m_segH < 0)       continue;

        int idx = ((k % n) + n) % n;
        if (!m_textures[idx]) continue;

        // Tile horizontally at m_tileW — SDL clips to logical viewport automatically
        for (float x = 0.f; x < (float)LOGICAL_W; x += m_tileW) {
            SDL_FRect dst = {x, y, m_tileW, m_segH};
            SDL_RenderTexture(renderer, m_textures[idx], nullptr, &dst);
        }
    }
}
