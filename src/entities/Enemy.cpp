#include "Enemy.h"
#include "../Constants.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, float y, EnemyType type, EnemyPattern pattern,
             SDL_Texture* tex, SDL_Texture* damageTex)
    : BaseEntity(x, y, 50, 38)
    , m_tex(tex), m_damageTex(damageTex)
    , m_type(type), m_pattern(pattern)
{
    m_velY    = ENEMY_SPEED_BASE + (std::rand() % 30);
    m_fireRate = ENEMY_FIRE_RATE + (std::rand() % 100) * 0.03f;

    switch (type) {
    case EnemyType::MEDIUM: m_hp = ENEMY_HP_MEDIUM; break;
    case EnemyType::LARGE:  m_hp = ENEMY_HP_LARGE;  break;
    default:                m_hp = ENEMY_HP_SMALL;  break;
    }
    m_maxHp = m_hp;

    if (tex) {
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);
        m_w = tw * PLANE_SCALE; m_h = th * PLANE_SCALE;
    }
}

int Enemy::scoreValue() const {
    switch (m_type) {
    case EnemyType::SMALL:  return SCORE_ENEMY_SMALL;
    case EnemyType::MEDIUM: return SCORE_ENEMY_MEDIUM;
    case EnemyType::UFO:    return SCORE_UFO;
    default:                return SCORE_ENEMY_SMALL;
    }
}

void Enemy::update(float dt) {
    if (!m_active) return;
    m_time += dt;

    switch (m_pattern) {
    case EnemyPattern::STRAIGHT:
        m_y += m_velY * dt;
        break;
    case EnemyPattern::SINE:
        m_y += m_velY * dt;
        m_x += std::sinf(m_time * 2.0f) * 80.0f * dt;
        break;
    case EnemyPattern::DIVE: {
        // Move toward formation target then dive straight down
        float dx = m_targetX - m_x;
        float dy = m_targetY - m_y;
        float dist = std::sqrtf(dx*dx + dy*dy);
        if (dist > 4.0f && !m_inFormation) {
            m_x += (dx / dist) * m_velY * dt;
            m_y += (dy / dist) * m_velY * dt;
        } else {
            m_inFormation = true;
            m_y += m_velY * dt;
        }
        break;
    }
    }

    // Fire timer
    m_fireTimer += dt;

    if (m_y > LOGICAL_H + m_h) m_active = false;
}

bool Enemy::hit() {
    if (--m_hp <= 0) {
        m_active = false;
        return true;
    }
    return false;
}

bool Enemy::tryFire(float /*dt*/) {
    if (m_fireTimer >= m_fireRate) {
        m_fireTimer = 0;
        return true;
    }
    return false;
}

void Enemy::render(SDL_Renderer* renderer) {
    if (!m_active || !m_tex) return;
    SDL_FRect dst = {m_x, m_y, m_w, m_h};
    SDL_RenderTexture(renderer, m_tex, nullptr, &dst);
}
