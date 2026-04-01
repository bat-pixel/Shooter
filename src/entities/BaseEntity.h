#pragma once
#include <SDL3/SDL.h>

class BaseEntity {
public:
    explicit BaseEntity(float x, float y, float w, float h);
    virtual ~BaseEntity() = default;

    virtual void update(float dt) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;

    SDL_FRect bounds() const { return {m_x, m_y, m_w, m_h}; }

    bool isActive() const { return m_active; }
    void setActive(bool v) { m_active = v; }

    bool collidesWithRect(const SDL_FRect& other) const;

protected:
    float m_x, m_y;
    float m_w, m_h;
    float m_velX = 0, m_velY = 0;
    bool  m_active = true;
};
