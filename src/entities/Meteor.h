#pragma once
#include "BaseEntity.h"
#include "../rendering/SpriteAnimation.h"
#include <SDL3/SDL.h>

enum class MeteorSize { TINY, SMALL, MEDIUM, BIG };

class Meteor : public BaseEntity {
public:
    Meteor(float x, float y, float velX, float velY,
           MeteorSize size, SDL_Texture* tex);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    MeteorSize size() const { return m_size; }

private:
    SDL_Texture* m_tex;
    MeteorSize   m_size;
    float        m_rotation = 0;
    float        m_rotSpeed;
};
