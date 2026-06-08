#include "Terrain.h"
#include "../Constants.h"

Terrain::Terrain(SDL_Texture* tex, float x, float scrollSpeed, int hp)
    : BaseEntity(x, -128.f, 64, 64)
    , m_tex(tex)
    , m_scrollSpeed(scrollSpeed)
    , m_hp(hp)
    , m_maxHp(hp)
{
    if (tex) {
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);
        m_w = tw * PLANE_SCALE;
        m_h = th * PLANE_SCALE;
        m_y = -m_h;
    }
    m_x = x - m_w * 0.5f;
}

void Terrain::update(float dt) {
    if (!m_active) return;
    m_y += m_scrollSpeed * dt;
    if (m_y > (float)LOGICAL_H)
        m_active = false;
}

bool Terrain::hit() {
    if (m_maxHp == 0) return false;
    if (--m_hp <= 0) {
        m_active = false;
        return true;
    }
    return false;
}

void Terrain::render(SDL_Renderer* renderer) {
    if (!m_active || !m_tex) return;
    SDL_FRect dst = {m_x, m_y, m_w, m_h};
    SDL_RenderTexture(renderer, m_tex, nullptr, &dst);
}
