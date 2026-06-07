#include "CollisionManager.h"
#include "../AudioManager.h"
#include "../Constants.h"

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
    auto& enemies = waves.enemies();
    for (auto& bullet : bullets.bullets()) {
        if (!bullet.isActive() || bullet.owner() != BulletOwner::PLAYER) continue;
        SDL_FRect br = bullet.bounds();

        for (int i = 0; i < (int)enemies.size(); ++i) {
            auto& enemy = enemies[i];
            if (!enemy->isActive()) continue;
            if (!enemy->collidesWithRect(br)) continue;

            bullet.setActive(false);
            if (enemy->hit()) {
                waves.onEnemyKilled(enemy.get(), i);
                if (m_scoreCb)     m_scoreCb(enemy->scoreValue());
                if (m_explosionCb) m_explosionCb(enemy->bounds().x, enemy->bounds().y);
                AudioManager::get().playSound("assets/sounds/explosion.mp3");
            }
            break;
        }

        // Player bullets vs meteors
        if (!bullet.isActive()) continue;
        for (auto& meteor : meteors) {
            if (!meteor->isActive()) continue;
            if (meteor->collidesWithRect(br)) {
                bullet.setActive(false);
                meteor->setActive(false);
                if (m_scoreCb)     m_scoreCb(SCORE_METEOR);
                if (m_explosionCb) m_explosionCb(meteor->bounds().x, meteor->bounds().y);
                AudioManager::get().playSound("assets/sounds/explosion.mp3");
                break;
            }
        }
    }

    // --- Enemy bullets vs player ---
    if (!player.isBulletFrozen()) {
        for (auto& bullet : bullets.bullets()) {
            if (!bullet.isActive() || bullet.owner() != BulletOwner::ENEMY) continue;
            if (player.collidesWithRect(bullet.bounds())) {
                bullet.setActive(false);
                player.hit();
                if (m_explosionCb) m_explosionCb(playerBounds.x, playerBounds.y);
            }
        }
    }

    // --- Enemies vs player (body collision) ---
    if (!player.isInvincible()) {
        for (int i = 0; i < (int)enemies.size(); ++i) {
            auto& enemy = enemies[i];
            if (!enemy->isActive()) continue;
            if (enemy->collidesWithRect(playerBounds)) {
                if (enemy->hit()) {
                    waves.onEnemyKilled(enemy.get(), i);
                    if (m_explosionCb) m_explosionCb(enemy->bounds().x, enemy->bounds().y);
                    AudioManager::get().playSound("assets/sounds/explosion.mp3");
                }
                player.hit();
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
        if (!pu->collidesWithRect(playerBounds)) continue;

        pu->setActive(false);
        PowerUpType t = pu->type();
        if (t == PowerUpType::SCORE_RED) {
            if (m_scoreCb) m_scoreCb(SCORE_POW_RED);
            AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        } else if (t == PowerUpType::YASHICHI) {
            if (m_scoreCb) m_scoreCb(SCORE_YASHICHI);
            AudioManager::get().playSound("assets/Bonus/sfx_twoTone.ogg");
        } else {
            player.applyPowerUp(t);
        }
    }
}
