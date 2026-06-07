#pragma once
#include "BaseEntity.h"
#include "PowerUp.h"
#include "../rendering/SpriteAnimation.h"
#include "../Constants.h"
#include <SDL3/SDL.h>
#include <array>

struct Wingman {
    float offsetX = 0;
    bool  active  = false;
};

class Player : public BaseEntity {
public:
    Player(float x, float y,
           SDL_Texture* shipTex,
           std::array<SDL_Texture*, 3> damageTex,
           SDL_Texture* wingmanTex = nullptr);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    bool wantsToFire();

    void hit();
    void doLoop();
    void applyPowerUp(PowerUpType type);
    void resetForNewStage();

    bool isInvincible()   const { return m_invincible; }
    bool isLooping()      const { return m_looping; }
    int  lives()          const { return m_lives; }
    int  shieldLevel()    const { return m_shieldLevel; }
    int  fireLevel()      const { return m_fireLevel; }
    int  loopsRemaining() const { return m_loopsRemaining; }
    bool hasScreenWipe()  const { return m_screenWipe; }
    bool isBulletFrozen() const { return m_bulletFreezeTimer > 0; }
    int  wingmanCount()   const;
    void clearScreenWipe()      { m_screenWipe = false; }

    const std::array<Wingman, 2>& wingmen() const { return m_wingmen; }

private:
    SDL_Texture*               m_shipTex;
    SDL_Texture*               m_wingmanTex   = nullptr;
    std::array<SDL_Texture*, 3> m_damageTex;
    std::array<Wingman, 2>     m_wingmen      = {};

    int   m_lives             = 3;
    float m_fireTimer         = 0;
    float m_invincibleTimer   = 0;
    bool  m_invincible        = false;

    bool  m_looping           = false;
    float m_loopTimer         = 0;
    float m_loopScale         = 1.0f;
    int   m_loopsRemaining    = PLAYER_START_LOOPS;

    int   m_shieldLevel       = 0;
    int   m_fireLevel         = 1;

    bool  m_screenWipe        = false;
    float m_bulletFreezeTimer = 0;
};
