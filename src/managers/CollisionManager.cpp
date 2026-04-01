#include "CollisionManager.h"
#include "../AudioManager.h"

void CollisionManager::setCallbacks(ScoreCb score, ExplosionCb explosion) {
    m_scoreCb     = std::move(score);
    m_explosionCb = std::move(explosion);
}

void CollisionManager::check(Player& player,
                              BulletManager& bullets,
                              EnemyWaveManager& waves,
                              std::vector<std::unique_ptr<Meteor>>& meteors) {
    SDL_FRect playerBounds = player.bounds();

    // --- Player bullets vs enemies ---
    for (auto& bullet : bullets.bullets()) {
        if (!bullet.isActive() || bullet.owner() != BulletOwner::PLAYER) continue;
        SDL_FRect br = bullet.bounds();

        for (auto& enemy : waves.enemies()) {
            if (!enemy->isActive()) continue;
            if (enemy->collidesWithRect(br)) {
                bullet.setActive(false);
                enemy->setActive(false);
                if (m_scoreCb)     m_scoreCb(enemy->scoreValue());
                if (m_explosionCb) m_explosionCb(enemy->bounds().x, enemy->bounds().y);
                AudioManager::get().playSound("assets/sounds/explosion.mp3");
            }
        }

        // Player bullets vs meteors
        for (auto& meteor : meteors) {
            if (!meteor->isActive()) continue;
            if (meteor->collidesWithRect(br)) {
                bullet.setActive(false);
                meteor->setActive(false);
                if (m_scoreCb)     m_scoreCb(SCORE_METEOR);
                if (m_explosionCb) m_explosionCb(meteor->bounds().x, meteor->bounds().y);
                AudioManager::get().playSound("assets/sounds/explosion.mp3");
            }
        }
    }

    // --- Enemy bullets vs player ---
    for (auto& bullet : bullets.bullets()) {
        if (!bullet.isActive() || bullet.owner() != BulletOwner::ENEMY) continue;
        if (player.collidesWithRect(bullet.bounds())) {
            bullet.setActive(false);
            player.hit();
            if (m_explosionCb) m_explosionCb(playerBounds.x, playerBounds.y);
        }
    }

    // --- Enemies vs player (body collision) ---
    if (!player.isInvincible()) {
        for (auto& enemy : waves.enemies()) {
            if (!enemy->isActive()) continue;
            if (enemy->collidesWithRect(playerBounds)) {
                enemy->setActive(false);
                player.hit();
                if (m_explosionCb) m_explosionCb(enemy->bounds().x, enemy->bounds().y);
                AudioManager::get().playSound("assets/sounds/explosion.mp3");
            }
        }
    }

    // --- Meteors vs player ---
    if (!player.isInvincible()) {
        for (auto& meteor : meteors) {
            if (!meteor->isActive()) continue;
            if (meteor->collidesWithRect(playerBounds)) {
                meteor->setActive(false);
                player.hit();
                if (m_explosionCb) m_explosionCb(playerBounds.x, playerBounds.y);
                AudioManager::get().playSound("assets/sounds/explosion.mp3");
            }
        }
    }

    // --- Power-ups vs player ---
    for (auto& pu : waves.powerUps()) {
        if (!pu->isActive()) continue;
        if (pu->collidesWithRect(playerBounds)) {
            player.applyPowerUp(pu->type());
            pu->setActive(false);
        }
    }
}
