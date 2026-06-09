#pragma once
#include "BaseEntity.h"

enum class BulletOwner { PLAYER, ENEMY };

class Bullet : public BaseEntity {
public:
    Bullet();   // pool: constructed inactive

    void fire(float x, float y, float velX, float velY,
              BulletOwner owner, SDL_Texture* tex, float scale = 1.0f);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    BulletOwner owner() const { return m_owner; }

private:
    SDL_Texture* m_tex    = nullptr;
    BulletOwner  m_owner  = BulletOwner::PLAYER;
};
