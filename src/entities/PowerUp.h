#pragma once
#include "BaseEntity.h"
#include <SDL3/SDL.h>

enum class PowerUpType { SHIELD, BOLT, STAR };

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
