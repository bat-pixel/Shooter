#include "BulletManager.h"
#include "../Constants.h"

BulletManager::BulletManager() {
    // Pool pre-initialised as inactive by Bullet default ctor
}

Bullet& BulletManager::getFree() {
    for (auto& b : m_pool)
        if (!b.isActive()) return b;
    // Pool exhausted — recycle oldest (first slot)
    return m_pool[0];
}

void BulletManager::spawnPlayer(float x, float y, int fireLevel,
                                SDL_Texture* tex) {
    // fireLevel 1: single centre; 2: two spread; 3: three spread
    auto fire = [&](float ox) {
        Bullet& b = getFree();
        b.fire(x + ox, y, 0, -PLAYER_BULLET_SPEED,
               BulletOwner::PLAYER, tex);
    };

    fire(0);
    if (fireLevel >= 2) { fire(-12.f); fire(12.f); }
    if (fireLevel >= 3) { fire(-24.f); fire(24.f); }
}

void BulletManager::spawnEnemy(float x, float y, float velX, float velY,
                               SDL_Texture* tex) {
    Bullet& b = getFree();
    b.fire(x, y, velX, velY, BulletOwner::ENEMY, tex);
}

void BulletManager::update(float dt) {
    for (auto& b : m_pool) b.update(dt);
}

void BulletManager::render(SDL_Renderer* renderer) {
    for (auto& b : m_pool) b.render(renderer);
}

void BulletManager::clear() {
    for (auto& b : m_pool) b.setActive(false);
}
