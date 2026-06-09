#include "PowerUp.h"
#include "../Constants.h"

PowerUp::PowerUp(float x, float y, PowerUpType type, SDL_Texture* tex)
    : BaseEntity(x, y, 48, 48), m_type(type), m_tex(tex) {
    m_velY = 80.0f;
    if (tex) {
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);
        m_w = tw * SPRITE_SCALE * 2.f;
        m_h = th * SPRITE_SCALE * 2.f;
    }
}

void PowerUp::update(float dt) {
    if (!m_active) return;
    m_y += m_velY * dt;
    if (m_y > LOGICAL_H + m_h) m_active = false;
}

void PowerUp::render(SDL_Renderer* renderer) {
    if (!m_active || !m_tex) return;
    SDL_FRect dst = {m_x, m_y, m_w, m_h};
    SDL_RenderTexture(renderer, m_tex, nullptr, &dst);
}
