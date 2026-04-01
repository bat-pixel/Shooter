#include "EnemyWaveManager.h"
#include "../Constants.h"
#include <cstdlib>

void EnemyWaveManager::loadTextures(SDL_Texture* black[5],
                                    SDL_Texture* red[5],
                                    SDL_Texture* blue[5],
                                    SDL_Texture* green[5],
                                    SDL_Texture* ufo) {
    for (int i = 0; i < 5; ++i) {
        m_enemyBlack[i] = black[i];
        m_enemyRed[i]   = red[i];
        m_enemyBlue[i]  = blue[i];
        m_enemyGreen[i] = green[i];
    }
    m_ufoTex = ufo;
}

void EnemyWaveManager::startWave(int waveNumber) {
    m_waveNumber = waveNumber;
    m_enemies.clear();

    int rows    = 2 + waveNumber / 3;
    int cols    = 5;
    int count   = rows * cols;
    m_aliveCount = count;

    EnemyPattern pat = (waveNumber % 3 == 0) ? EnemyPattern::DIVE
                     : (waveNumber % 2 == 0) ? EnemyPattern::SINE
                     :                         EnemyPattern::STRAIGHT;

    // Choose texture set based on wave mod 4
    SDL_Texture** set = m_enemyRed;
    switch (waveNumber % 4) {
    case 0: set = m_enemyBlack; break;
    case 1: set = m_enemyRed;   break;
    case 2: set = m_enemyBlue;  break;
    case 3: set = m_enemyGreen; break;
    }

    float spacingX = (LOGICAL_W - 60.f) / cols;
    float spacingY = 48.f;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float tx = 30.f + c * spacingX;
            float ty = -50.f - r * spacingY;
            int   tidx = std::min(r % 5, 4);
            auto e = std::make_unique<Enemy>(
                tx, ty - 200.f,
                EnemyType::SMALL, pat,
                set[tidx]);
            e->setFormationTarget(tx, ty);
            m_enemies.push_back(std::move(e));
        }
    }

    // Occasionally add a UFO on higher waves
    if (waveNumber > 3 && m_ufoTex) {
        float ux = 30.f + (std::rand() % (LOGICAL_W - 60));
        auto u = std::make_unique<Enemy>(
            ux, -60.f, EnemyType::UFO, EnemyPattern::SINE, m_ufoTex);
        m_enemies.push_back(std::move(u));
        ++m_aliveCount;
    }
}

void EnemyWaveManager::update(float dt) {
    for (auto& e : m_enemies) e->update(dt);
    for (auto& p : m_powerUps) p->update(dt);

    // Remove power-ups that fell off screen
    m_powerUps.erase(
        std::remove_if(m_powerUps.begin(), m_powerUps.end(),
            [](const auto& p){ return !p->isActive(); }),
        m_powerUps.end());
}

void EnemyWaveManager::render(SDL_Renderer* renderer) {
    for (auto& e : m_enemies) e->render(renderer);
    for (auto& p : m_powerUps) p->render(renderer);
}

void EnemyWaveManager::clear() {
    m_enemies.clear();
    m_powerUps.clear();
    m_aliveCount = 0;
}

bool EnemyWaveManager::isWaveCleared() const {
    for (const auto& e : m_enemies)
        if (e->isActive()) return false;
    return true;
}

void EnemyWaveManager::onEnemyKilled(Enemy* /*e*/, int /*idx*/) {
    --m_aliveCount;
}
