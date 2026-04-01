#include "Player.h"
#include "../InputManager.h"
#include "../Constants.h"
#include "../AudioManager.h"
#include <cmath>
#include <algorithm>

Player::Player(float x, float y,
               SDL_Texture* shipTex,
               std::array<SDL_Texture*, 3> damageTex)
    : BaseEntity(x, y, 66, 75)
    , m_shipTex(shipTex)
    , m_damageTex(damageTex)
{
    if (shipTex) {
        float tw, th;
        SDL_GetTextureSize(shipTex, &tw, &th);
        m_w = tw; m_h = th;
    }
}

void Player::update(float dt) {
    if (!m_active) return;
    auto& input = InputManager::get();

    // Movement
    float ax = input.axisX();
    float ay = input.axisY();
    m_x += ax * PLAYER_SPEED * dt;
    m_y += ay * PLAYER_SPEED * dt;

    // Clamp to screen
    m_x = std::clamp(m_x, 0.f, (float)LOGICAL_W - m_w);
    m_y = std::clamp(m_y, 0.f, (float)LOGICAL_H - m_h);

    // Fire cooldown
    m_fireTimer -= dt;

    // Loop-the-loop
    if (input.isPressed(Action::LOOP) && !m_looping) {
        doLoop();
    }

    if (m_looping) {
        m_loopTimer += dt;
        // Scale: shrink then grow back
        float t = m_loopTimer / LOOP_DURATION;
        m_loopScale = 1.0f - 0.5f * std::sinf(t * 3.14159f);
        if (m_loopTimer >= LOOP_DURATION) {
            m_looping       = false;
            m_loopScale     = 1.0f;
            m_invincible    = false;
            m_invincibleTimer = 0;
        }
    }

    // Invincibility (from hit)
    if (m_invincible && !m_looping) {
        m_invincibleTimer -= dt;
        if (m_invincibleTimer <= 0) {
            m_invincible      = false;
            m_invincibleTimer = 0;
        }
    }
}

bool Player::wantsToFire() {
    if (m_fireTimer > 0) return false;
    if (!InputManager::get().isHeld(Action::FIRE)) return false;
    m_fireTimer = PLAYER_FIRE_RATE;
    return true;
}

void Player::hit() {
    if (m_invincible) return;
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
    AudioManager::get().playSound("assets/sounds/loop.mp3");
}

void Player::applyPowerUp(PowerUpType type) {
    switch (type) {
    case PowerUpType::SHIELD:
        m_shieldLevel = std::min(m_shieldLevel + 1, 3);
        AudioManager::get().playSound("assets/Bonus/sfx_shieldUp.ogg");
        break;
    case PowerUpType::BOLT:
        m_fireLevel = std::min(m_fireLevel + 1, 3);
        AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        break;
    case PowerUpType::STAR:
        m_lives = std::min(m_lives + 1, PLAYER_MAX_LIVES);
        AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        break;
    }
}

void Player::render(SDL_Renderer* renderer) {
    if (!m_active) return;

    // Blink when hit-invincible
    if (m_invincible && !m_looping) {
        Uint64 ticks = SDL_GetTicks();
        if ((ticks / 100) % 2 == 0) return;   // blink every 100ms
    }

    // Pick damage texture based on lives
    SDL_Texture* tex = m_shipTex;
    if (m_lives == 2 && m_damageTex[0]) tex = m_damageTex[0];
    if (m_lives == 1 && m_damageTex[1]) tex = m_damageTex[1];

    float rw = m_w * m_loopScale;
    float rh = m_h * m_loopScale;
    float rx = m_x + (m_w - rw) * 0.5f;
    float ry = m_y + (m_h - rh) * 0.5f;

    SDL_FRect dst = {rx, ry, rw, rh};
    SDL_RenderTexture(renderer, tex, nullptr, &dst);

    // Shield ring overlay
    // (rendered by Game using Effects/shield textures)
}
