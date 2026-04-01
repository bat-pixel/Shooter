#pragma once
#include "../entities/Bullet.h"
#include <array>
#include <SDL3/SDL.h>
#include "../Constants.h"

class BulletManager {
public:
    BulletManager();

    void spawnPlayer(float x, float y, int fireLevel,
                     SDL_Texture* tex);
    void spawnEnemy(float x, float y, float velX, float velY,
                    SDL_Texture* tex);

    void update(float dt);
    void render(SDL_Renderer* renderer);
    void clear();

    // Iterating for collision checks
    std::array<Bullet, BULLET_POOL_SIZE>& bullets() { return m_pool; }

private:
    Bullet& getFree();

    std::array<Bullet, BULLET_POOL_SIZE> m_pool;
};
