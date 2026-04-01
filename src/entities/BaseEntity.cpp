#include "BaseEntity.h"

BaseEntity::BaseEntity(float x, float y, float w, float h)
    : m_x(x), m_y(y), m_w(w), m_h(h) {}

bool BaseEntity::collidesWithRect(const SDL_FRect& other) const {
    SDL_FRect self = bounds();
    return SDL_HasRectIntersectionFloat(&self, &other);
}
