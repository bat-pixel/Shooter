#include "Boss.h"
#include "../Constants.h"
#include <cmath>
#include <algorithm>

Boss::Boss(SDL_Texture* tex, int bossIndex, bool flipV)
    : BaseEntity(0, -160.f, 128, 128)
    , m_tex(tex)
    , m_bossIndex(std::clamp(bossIndex, 1, 8))
    , m_flipV(flipV)
{
    if (tex) {
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);
        m_w = tw * PLANE_SCALE;
        m_h = th * PLANE_SCALE;
    }
    m_x = LOGICAL_W * 0.5f - m_w * 0.5f;

    int scaledHp = 50 + m_bossIndex * 30;  // 80 (boss1) → 290 (boss8)
    m_hp    = scaledHp;
    m_maxHp = scaledHp;
}

void Boss::update(float dt) {
    if (!m_active) return;
    m_time += dt;

    // Entry glide
    if (m_y < m_targetY) {
        m_y += 80.f * dt;
        return;
    }

    // Aggressive sidestep: fast sinusoidal sweep across most of the screen
    float freq  = 0.9f + m_bossIndex * 0.06f;
    float sweep = (LOGICAL_W - m_w) * 0.48f;
    m_x = LOGICAL_W * 0.5f - m_w * 0.5f + std::sinf(m_time * freq) * sweep;
    m_x = std::clamp(m_x, 0.f, (float)LOGICAL_W - m_w);

    m_fireTimer += dt;
}

bool Boss::tryFire(float playerX, float playerY,
                   float& outX, float& outY, float& outVx, float& outVy) {
    // Fire rate tiers: normal → enraged → frantic
    float fireRate;
    if (m_hp <= m_maxHp / 4)       fireRate = 0.18f;
    else if (m_hp <= m_maxHp / 2)  fireRate = 0.28f;
    else                            fireRate = 0.45f;

    // Later bosses fire faster than earlier ones. Ayako II (index 2) is noticeably
    // quicker than the first boss, and the trend continues through the campaign.
    float rateScale = std::max(0.55f, 1.0f - (m_bossIndex - 1) * 0.07f);
    fireRate *= rateScale;

    if (m_fireTimer < fireRate) return false;
    m_fireTimer = 0;

    float cx = m_x + m_w * 0.5f;
    float cy = m_y + m_h;

    outX = cx;
    outY = cy;

    int phase = m_shotPhase % 4;
    ++m_shotPhase;

    if (phase == 0 || phase == 3) {
        // Aimed at player
        float dx = playerX - cx;
        float dy = playerY - cy;
        float len = std::sqrtf(dx * dx + dy * dy);
        if (len < 1.f) len = 1.f;
        outVx = (dx / len) * ENEMY_BULLET_SPEED;
        outVy = (dy / len) * ENEMY_BULLET_SPEED;
    } else if (phase == 1) {
        // 3-way fan: slight left bias — Game.cpp mirrors to right and adds center
        outVx = -0.3f * ENEMY_BULLET_SPEED;
        outVy =  0.95f * ENEMY_BULLET_SPEED;
    } else {
        // Wide spread: ±50° — Game.cpp mirrors and adds center
        outVx = -0.6f * ENEMY_BULLET_SPEED;
        outVy =  0.8f * ENEMY_BULLET_SPEED;
    }

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
    SDL_RenderTextureRotated(renderer, m_tex, nullptr, &dst, 0.0, nullptr,
                             m_flipV ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

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
