#pragma once
#include "BaseEntity.h"
#include "PowerUp.h"
#include "../rendering/SpriteAnimation.h"
#include <SDL3/SDL.h>
#include <array>

class Player : public BaseEntity {
public:
    Player(float x, float y,
           SDL_Texture* shipTex,
           std::array<SDL_Texture*, 3> damageTex);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    // Returns true if player fired this frame
    bool wantsToFire();

    void hit();           // take one hit (respects invincibility)
    void doLoop();        // trigger loop-the-loop
    void applyPowerUp(PowerUpType type);

    bool  isInvincible()   const { return m_invincible; }
    bool  isLooping()      const { return m_looping; }
    int   lives()          const { return m_lives; }
    int   shieldLevel()    const { return m_shieldLevel; }
    int   fireLevel()      const { return m_fireLevel; }

private:
    SDL_Texture*               m_shipTex;
    std::array<SDL_Texture*, 3> m_damageTex;

    int   m_lives         = 3;
    float m_fireTimer     = 0;
    float m_invincibleTimer = 0;
    bool  m_invincible    = false;

    bool  m_looping       = false;
    float m_loopTimer     = 0;
    float m_loopScale     = 1.0f;

    int   m_shieldLevel   = 0;   // 0=none, 1-3
    int   m_fireLevel     = 1;   // 1=single, 2=double, 3=triple
};
