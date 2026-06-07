#pragma once
#include "BaseEntity.h"
#include <SDL3/SDL.h>

class Boss : public BaseEntity {
public:
    explicit Boss(SDL_Texture* tex);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    bool tryFire(float& outX, float& outY, float& outVx, float& outVy);
    bool hit();

    int hp()    const { return m_hp; }
    int maxHp() const { return m_maxHp; }

private:
    SDL_Texture* m_tex      = nullptr;
    int          m_hp       = 30;
    int          m_maxHp    = 30;
    float        m_time     = 0;
    float        m_fireTimer = 0;
    float        m_targetY   = 60.f;
    int          m_shotPhase = 0;
};
