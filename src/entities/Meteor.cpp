#include "Meteor.h"
#include "../Constants.h"
#include <SDL3/SDL.h>
#include <cstdlib>

static float sizeW(MeteorSize s) {
    switch(s) {
    case MeteorSize::BIG:    return 101;
    case MeteorSize::MEDIUM: return 43;
    case MeteorSize::SMALL:  return 28;
    default:                 return 18;
    }
}

Meteor::Meteor(float x, float y, float velX, float velY,
               MeteorSize size, SDL_Texture* tex)
    : BaseEntity(x, y, sizeW(size), sizeW(size))
    , m_tex(tex), m_size(size)
{
    m_velX = velX;
    m_velY = velY;
    m_rotSpeed = 30.0f + (std::rand() % 120);
    if (std::rand() % 2) m_rotSpeed = -m_rotSpeed;

    if (tex) {
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);
        m_w = tw * SPRITE_SCALE; m_h = th * SPRITE_SCALE;
    }
}

void Meteor::update(float dt) {
    if (!m_active) return;
    m_x        += m_velX * dt;
    m_y        += m_velY * dt;
    m_rotation += m_rotSpeed * dt;

    if (m_y > LOGICAL_H + m_h) m_active = false;
}

void Meteor::render(SDL_Renderer* renderer) {
    if (!m_active || !m_tex) return;
    SDL_FRect dst = {m_x, m_y, m_w, m_h};
    SDL_RenderTextureRotated(renderer, m_tex, nullptr, &dst,
                             m_rotation, nullptr, SDL_FLIP_NONE);
}
