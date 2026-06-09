#include "Enemy.h"
#include "../Constants.h"
#include <cmath>
#include <cstdlib>

static constexpr float PI = 3.14159265f;
static constexpr float LOOP_RADIUS = 50.f;

Enemy::Enemy(float x, float y, EnemyType type, EnemyPattern pattern,
             SDL_Texture* tex, SDL_Texture* damageTex, float speedMult)
    : BaseEntity(x, y, 50, 38)
    , m_tex(tex), m_damageTex(damageTex)
    , m_type(type), m_pattern(pattern)
    , m_initialX(x)
{
    m_velY    = (ENEMY_SPEED_BASE + (std::rand() % 30)) * speedMult;
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
        m_renderAngle = 0.f;
        break;

    case EnemyPattern::SINE:
        m_y += m_velY * dt;
        m_x  = m_initialX + std::sinf(m_time * 2.5f) * 55.0f;
        m_renderAngle = 0.f;
        break;

    case EnemyPattern::DIVE: {
        if (!m_inFormation) {
            // Phase 1 — fly to formation target
            float dx   = m_targetX - m_x;
            float dy   = m_targetY - m_y;
            float dist = std::sqrtf(dx*dx + dy*dy);
            if (dist > 4.0f) {
                m_x += (dx / dist) * m_velY * dt;
                m_y += (dy / dist) * m_velY * dt;
                m_renderAngle = std::atan2f(dx, dy) * (180.f / PI);
            } else {
                m_x = m_targetX;
                m_y = m_targetY;
                m_inFormation = true;
                m_renderAngle = 0.f;
            }
        } else if (!m_looping && !m_diving) {
            // Phase 2 — hold formation briefly
            m_formationTimer += dt;
            m_renderAngle = 0.f;
            if (m_formationTimer >= 0.8f) {
                m_looping     = true;
                m_loopCenterX = m_x;
                m_loopCenterY = m_y + 45.0f;
                m_loopAngle   = -PI * 0.5f;
            }
        } else if (m_looping) {
            // Phase 3 — half-circle loop
            float loopSpeed = 3.5f;
            float radius    = 45.0f;
            m_loopAngle += loopSpeed * dt;
            m_x = m_loopCenterX + std::cosf(m_loopAngle) * radius;
            m_y = m_loopCenterY + std::sinf(m_loopAngle) * radius;
            // Tangent velocity direction for sprite rotation
            float tvx = -std::sinf(m_loopAngle) * loopSpeed;
            float tvy =  std::cosf(m_loopAngle) * loopSpeed;
            m_renderAngle = std::atan2f(tvx, tvy) * (180.f / PI);
            if (m_loopAngle >= PI * 0.5f) {
                m_looping = false;
                m_diving  = true;
                m_renderAngle = 0.f;
            }
        } else {
            // Phase 4 — post-loop straight dive
            m_y += m_velY * 1.8f * dt;
            m_renderAngle = 0.f;
        }
        break;
    }

    case EnemyPattern::ARC: {
        // Phase 0: one-time init — set diagonal velocity toward center
        if (m_phase == 0) {
            bool fromLeft = m_initialX < LOGICAL_W * 0.5f;
            m_arcForce = fromLeft ? -1.f : 1.f;           // curve direction in phase 2
            m_velX     = fromLeft ? m_velY * 0.65f : -m_velY * 0.65f;
            m_phase    = 1;
        }
        m_phaseTimer += dt;
        if (m_phase == 1) {
            // Diagonal approach until visibly on-screen
            m_y += m_velY * dt;
            m_x += m_velX * dt;
            m_renderAngle = std::atan2f(m_velX, m_velY) * (180.f / PI);
            if (m_y >= 60.f) m_phase = 2;
        } else {
            // Phase 2: parabolic curve — accelerate back toward spawn side
            float accel = m_velY * 2.2f;
            m_velX += m_arcForce * accel * dt;
            m_velY *= (1.f - 0.35f * dt);  // bleed vertical speed as bank deepens
            m_y += m_velY * dt;
            m_x += m_velX * dt;
            m_renderAngle = std::atan2f(m_velX, m_velY) * (180.f / PI);
            if (m_x < -m_w * 2.f || m_x > LOGICAL_W + m_w * 2.f) m_active = false;
        }
        break;
    }

    case EnemyPattern::LOOP_DIVE: {
        if (m_phase == 0) {
            // Straight down entry until reaching loop trigger depth
            m_y += m_velY * dt;
            m_renderAngle = 0.f;
            if (m_y >= 80.f) {
                m_phase       = 1;
                m_loopCenterX = m_x;
                m_loopCenterY = m_y + LOOP_RADIUS;
                m_loopAngle   = -PI * 0.5f;
            }
        } else if (m_phase == 1) {
            // Full 360° vertical loop — speed varies: faster at bottom, slower at top
            float baseRate  = m_velY / LOOP_RADIUS;
            float loopSpeed = baseRate * (1.3f + 0.35f * std::sinf(m_loopAngle));
            m_loopAngle += loopSpeed * dt;
            m_x = m_loopCenterX + std::cosf(m_loopAngle) * LOOP_RADIUS;
            m_y = m_loopCenterY + std::sinf(m_loopAngle) * LOOP_RADIUS;
            // Tangent direction for sprite rotation
            float tvx = -std::sinf(m_loopAngle) * loopSpeed;
            float tvy =  std::cosf(m_loopAngle) * loopSpeed;
            m_renderAngle = std::atan2f(tvx, tvy) * (180.f / PI);
            // Full circle: from -π/2 back to -π/2 + 2π = 3π/2
            if (m_loopAngle >= PI * 1.5f) {
                m_phase = 2;
                // Aimed aggressive dive toward player
                float dx  = m_playerX - m_x;
                float dy  = m_playerY - m_y;
                float len = std::sqrtf(dx*dx + dy*dy);
                if (len < 1.f) { dx = 0; dy = 1; len = 1; }
                float diveSpeed = m_velY * 1.9f;
                m_diveVelX = (dx / len) * diveSpeed;
                m_diveVelY = (dy / len) * diveSpeed;
                // Ensure always moving downward
                if (m_diveVelY < m_velY * 0.6f) {
                    m_diveVelY = m_velY * 1.5f;
                    m_diveVelX *= 0.5f;
                }
                m_renderAngle = std::atan2f(m_diveVelX, m_diveVelY) * (180.f / PI);
            }
        } else {
            // Phase 2: dive-bomb toward player
            m_x += m_diveVelX * dt;
            m_y += m_diveVelY * dt;
            m_renderAngle = std::atan2f(m_diveVelX, m_diveVelY) * (180.f / PI);
        }
        break;
    }
    } // end switch

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
    if (m_renderAngle != 0.f) {
        SDL_RenderTextureRotated(renderer, m_tex, nullptr, &dst,
                                 (double)m_renderAngle, nullptr, SDL_FLIP_NONE);
    } else {
        SDL_RenderTexture(renderer, m_tex, nullptr, &dst);
    }
}
