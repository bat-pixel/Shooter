#pragma once
#include "BaseEntity.h"
#include <SDL3/SDL.h>

enum class PowerUpType {
    DOUBLE_SHOT,    // Green POW  — widens fire range
    SCREEN_WIPE,    // White POW  — clears all enemies
    WINGMAN,        // Gray POW   — adds side fighters
    FREEZE_BULLETS, // Orange POW — stops enemy fire 3s
    EXTRA_LOOP,     // Yellow POW — +1 roll
    EXTRA_LIFE,     // Black POW  — +1 life (rare)
    SCORE_RED,      // Red POW   — 1000 pts
    YASHICHI,       // Dragonfly  — 5000 pts
};

class PowerUp : public BaseEntity {
public:
    PowerUp(float x, float y, PowerUpType type, SDL_Texture* tex);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    PowerUpType type() const { return m_type; }

private:
    PowerUpType  m_type;
    SDL_Texture* m_tex;
};
