#include "Boss.h"
#include "../Constants.h"
#include <cmath>
#include <algorithm>

Boss::Boss(SDL_Texture* tex)
    : BaseEntity(0, -160.f, 128, 128)
    , m_tex(tex)
{
    if (tex) {
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);
        m_w = tw * PLANE_SCALE;
        m_h = th * PLANE_SCALE;
    }
    m_x     = LOGICAL_W * 0.5f - m_w * 0.5f;
    m_hp    = 30;
    m_maxHp = 30;
}

void Boss::update(float dt) {
    if (!m_active) return;
    m_time += dt;

    // Entry glide
    if (m_y < m_targetY) {
        m_y += 80.f * dt;
        return;
    }

    // Slow sinusoidal patrol across top quarter of screen
    float sweep = (LOGICAL_W - m_w) * 0.45f;
    m_x = LOGICAL_W * 0.5f - m_w * 0.5f + std::sinf(m_time * 0.6f) * sweep;
    m_x = std::clamp(m_x, 0.f, (float)LOGICAL_W - m_w);

    m_fireTimer += dt;
}

bool Boss::tryFire(float& outX, float& outY, float& outVx, float& outVy) {
    float fireRate = (m_hp < m_maxHp / 2) ? 0.6f : 1.0f;
    if (m_fireTimer < fireRate) return false;
    m_fireTimer = 0;

    float cx = m_x + m_w * 0.5f;
    float cy = m_y + m_h;

    // Alternate between spread patterns
    static const float patterns[3][2] = {{0,-1}, {-0.3f,-0.95f}, {0.3f,-0.95f}};
    int p = m_shotPhase % 3;
    outX  = cx;
    outY  = cy;
    outVx = patterns[p][0] * ENEMY_BULLET_SPEED;
    outVy = -patterns[p][1] * ENEMY_BULLET_SPEED;
    ++m_shotPhase;
    return true;
}

bool Boss::hit() {
    if (--m_hp <= 0) {
        m_active = false;
        return true;
    }
    return false;
}

void Boss::render(SDL_Renderer* renderer) {
    if (!m_active || !m_tex) return;
    SDL_FRect dst = {m_x, m_y, m_w, m_h};
    SDL_RenderTexture(renderer, m_tex, nullptr, &dst);

    // Health bar just above boss
    float barW = m_w;
    float barY = m_y - 8.f;
    SDL_SetRenderDrawColor(renderer, 80, 0, 0, 200);
    SDL_FRect bgBar = {m_x, barY, barW, 5.f};
    SDL_RenderFillRect(renderer, &bgBar);
    SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
    SDL_FRect hpBar = {m_x, barY, barW * (float)m_hp / m_maxHp, 5.f};
    SDL_RenderFillRect(renderer, &hpBar);
}
