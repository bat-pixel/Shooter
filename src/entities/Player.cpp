#include "Player.h"
#include "../InputManager.h"
#include "../Constants.h"
#include "../AudioManager.h"
#include <cmath>
#include <algorithm>

Player::Player(float x, float y,
               SDL_Texture* shipTex,
               std::array<SDL_Texture*, 3> damageTex,
               SDL_Texture* wingmanTex)
    : BaseEntity(x, y, 66, 75)
    , m_shipTex(shipTex)
    , m_damageTex(damageTex)
    , m_wingmanTex(wingmanTex)
{
    if (shipTex) {
        float tw, th;
        SDL_GetTextureSize(shipTex, &tw, &th);
        m_w = tw * SPRITE_SCALE; m_h = th * SPRITE_SCALE;
    }
    m_wingmen[0].offsetX = -30.f;
    m_wingmen[1].offsetX =  30.f;
}

void Player::update(float dt) {
    if (!m_active) return;
    auto& input = InputManager::get();

    float ax = input.axisX();
    float ay = input.axisY();
    m_x += ax * PLAYER_SPEED * dt;
    m_y += ay * PLAYER_SPEED * dt;
    m_x = std::clamp(m_x, 0.f, (float)LOGICAL_W - m_w);
    m_y = std::clamp(m_y, 0.f, (float)LOGICAL_H - m_h);

    m_fireTimer -= dt;

    if (input.isPressed(Action::LOOP) && !m_looping && m_loopsRemaining > 0) {
        doLoop();
    }

    if (m_looping) {
        m_loopTimer += dt;
        float t = m_loopTimer / LOOP_DURATION;
        m_loopScale = 1.0f - 0.5f * std::sinf(t * 3.14159f);
        if (m_loopTimer >= LOOP_DURATION) {
            m_looping         = false;
            m_loopScale       = 1.0f;
            m_invincible      = false;
            m_invincibleTimer = 0;
        }
    }

    if (m_invincible && !m_looping) {
        m_invincibleTimer -= dt;
        if (m_invincibleTimer <= 0) {
            m_invincible      = false;
            m_invincibleTimer = 0;
        }
    }

    if (m_bulletFreezeTimer > 0)
        m_bulletFreezeTimer -= dt;
}

bool Player::wantsToFire() {
    if (m_fireTimer > 0) return false;
    if (!InputManager::get().isHeld(Action::FIRE)) return false;
    m_fireTimer = PLAYER_FIRE_RATE;
    return true;
}

void Player::hit() {
    if (m_invincible || m_godMode) return;
    if (m_shieldLevel > 0) {
        --m_shieldLevel;
        AudioManager::get().playSound("assets/Bonus/sfx_shieldDown.ogg");
        return;
    }
    --m_lives;
    if (m_lives <= 0) {
        m_active = false;
        return;
    }
    m_invincible      = true;
    m_invincibleTimer = 2.0f;
}

void Player::doLoop() {
    m_looping        = true;
    m_loopTimer      = 0;
    m_invincible     = true;
    --m_loopsRemaining;
    AudioManager::get().playSound("assets/sounds/loop.mp3");
}

void Player::applyPowerUp(PowerUpType type) {
    switch (type) {
    case PowerUpType::DOUBLE_SHOT:
        m_fireLevel = std::min(m_fireLevel + 1, 3);
        AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        break;
    case PowerUpType::SCREEN_WIPE:
        m_screenWipe = true;
        AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        break;
    case PowerUpType::WINGMAN:
        if (!m_wingmen[0].active)      m_wingmen[0].active = true;
        else if (!m_wingmen[1].active) m_wingmen[1].active = true;
        AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        break;
    case PowerUpType::FREEZE_BULLETS:
        m_bulletFreezeTimer = 3.0f;
        AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        break;
    case PowerUpType::EXTRA_LOOP:
        ++m_loopsRemaining;
        AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        break;
    case PowerUpType::EXTRA_LIFE:
        m_lives = std::min(m_lives + 1, PLAYER_MAX_LIVES);
        AudioManager::get().playSound("assets/Bonus/sfx_shieldUp.ogg");
        break;
    case PowerUpType::SCORE_RED:
    case PowerUpType::YASHICHI:
        // Score handled by CollisionManager callback
        AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        break;
    }
}

int Player::wingmanCount() const {
    int n = 0;
    for (const auto& wm : m_wingmen) if (wm.active) ++n;
    return n;
}

void Player::resetForNewStage() {
    m_loopsRemaining    = PLAYER_START_LOOPS;
    m_fireLevel         = 1;
    m_shieldLevel       = 0;
    m_bulletFreezeTimer = 0;
    m_screenWipe        = false;
    m_wingmen[0].active = false;
    m_wingmen[1].active = false;
}

void Player::render(SDL_Renderer* renderer) {
    if (!m_active) return;

    if (m_invincible && !m_looping) {
        Uint64 ticks = SDL_GetTicks();
        if ((ticks / 100) % 2 == 0) return;
    }

    SDL_Texture* tex = m_shipTex;
    if (m_lives == 2 && m_damageTex[0]) tex = m_damageTex[0];
    if (m_lives == 1 && m_damageTex[1]) tex = m_damageTex[1];

    float rw = m_w * m_loopScale;
    float rh = m_h * m_loopScale;
    float rx = m_x + (m_w - rw) * 0.5f;
    float ry = m_y + (m_h - rh) * 0.5f;

    SDL_FRect dst = {rx, ry, rw, rh};
    SDL_RenderTexture(renderer, tex, nullptr, &dst);

    // Wingmen
    if (m_wingmanTex) {
        float ww, wh;
        SDL_GetTextureSize(m_wingmanTex, &ww, &wh);
        ww *= SPRITE_SCALE; wh *= SPRITE_SCALE;
        for (const auto& wm : m_wingmen) {
            if (!wm.active) continue;
            SDL_FRect wd = {m_x + m_w * 0.5f + wm.offsetX - ww * 0.5f,
                            m_y + m_h * 0.5f - wh * 0.5f, ww, wh};
            SDL_RenderTexture(renderer, m_wingmanTex, nullptr, &wd);
        }
    }
}
