#include "Background.h"
#include "../Constants.h"

Background::Background(SDL_Texture* tex, float scrollSpeed)
    : m_tex(tex), m_speed(scrollSpeed) {
    if (tex) SDL_GetTextureSize(tex, &m_texW, &m_texH);
}

void Background::update(float dt) {
    m_offsetY += m_speed * dt;
    if (m_texH > 0 && m_offsetY >= m_texH) m_offsetY -= m_texH;
}

void Background::setTexture(SDL_Texture* tex) {
    m_tex     = tex;
    m_offsetY = 0;
    if (tex) SDL_GetTextureSize(tex, &m_texW, &m_texH);
}

void Background::render(SDL_Renderer* renderer) {
    if (!m_tex) return;

    // Tile vertically: draw two copies so the seam is invisible
    float srcH = m_texH > 0 ? m_texH : LOGICAL_H;

    SDL_FRect dst1 = {0, m_offsetY - srcH, (float)LOGICAL_W, srcH};
    SDL_FRect dst2 = {0, m_offsetY,        (float)LOGICAL_W, srcH};

    SDL_RenderTexture(renderer, m_tex, nullptr, &dst1);
    SDL_RenderTexture(renderer, m_tex, nullptr, &dst2);
}
