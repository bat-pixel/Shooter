#pragma once
#include "BaseEntity.h"
#include <SDL3/SDL.h>

class Terrain : public BaseEntity {
public:
    Terrain(SDL_Texture* tex, float x, float scrollSpeed, int hp = 0);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    bool hit();
    bool isDestructible() const { return m_maxHp > 0; }

private:
    SDL_Texture* m_tex         = nullptr;
    float        m_scrollSpeed = 80.f;
    int          m_hp          = 0;
    int          m_maxHp       = 0;
};
