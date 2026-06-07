#include "Bullet.h"
#include "../Constants.h"
#include <SDL3/SDL.h>

Bullet::Bullet() : BaseEntity(0, 0, 9, 37) {
    m_active = false;
}

void Bullet::fire(float x, float y, float velX, float velY,
                  BulletOwner owner, SDL_Texture* tex) {
    m_x     = x;
    m_y     = y;
    m_velX  = velX;
    m_velY  = velY;
    m_owner = owner;
    m_tex   = tex;
    m_active = true;

    // Size from texture
    if (tex) {
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);
        m_w = tw * PLANE_SCALE; m_h = th * PLANE_SCALE;
    }
}

void Bullet::update(float dt) {
    if (!m_active) return;
    m_x += m_velX * dt;
    m_y += m_velY * dt;

    if (m_y < -m_h || m_y > LOGICAL_H + m_h ||
        m_x < -m_w || m_x > LOGICAL_W + m_w) {
        m_active = false;
    }
}

void Bullet::render(SDL_Renderer* renderer) {
    if (!m_active || !m_tex) return;
    SDL_FRect dst = {m_x, m_y, m_w, m_h};
    SDL_RenderTexture(renderer, m_tex, nullptr, &dst);
}
