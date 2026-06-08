#pragma once
#include "BaseEntity.h"
#include <SDL3/SDL.h>

class Boss : public BaseEntity {
public:
    explicit Boss(SDL_Texture* tex, int bossIndex = 1);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    // playerX/Y used for aimed shots; fills one bullet direction per call
    bool tryFire(float playerX, float playerY,
                 float& outX, float& outY, float& outVx, float& outVy);
    bool hit();

    int hp()    const { return m_hp; }
    int maxHp() const { return m_maxHp; }

private:
    SDL_Texture* m_tex       = nullptr;
    int          m_bossIndex = 1;
    int          m_hp        = 80;
    int          m_maxHp     = 80;
    float        m_time      = 0;
    float        m_fireTimer = 0;
    float        m_targetY   = 60.f;
    int          m_shotPhase = 0;
};
