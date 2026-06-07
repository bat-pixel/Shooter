#include "EnemyWaveManager.h"
#include "../Constants.h"
#include "../AssetManager.h"
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
    m_formations.clear();
    m_enemyFormation.clear();
    m_powerUps.clear();

    int rows  = 2 + waveNumber / 3;
    int cols  = 5;
    m_aliveCount   = rows * cols;
    m_totalSpawned = m_aliveCount;

    EnemyPattern pat = (waveNumber % 3 == 0) ? EnemyPattern::DIVE
                     : (waveNumber % 2 == 0) ? EnemyPattern::SINE
                     :                         EnemyPattern::STRAIGHT;

    SDL_Texture** set = m_enemyRed;
    switch (waveNumber % 4) {
    case 0: set = m_enemyBlack; break;
    case 1: set = m_enemyRed;   break;
    case 2: set = m_enemyBlue;  break;
    case 3: set = m_enemyGreen; break;
    }

    float spacingX = (LOGICAL_W - 60.f) / cols;
    float spacingY = 48.f;

    static const PowerUpType powCycle[] = {
        PowerUpType::DOUBLE_SHOT,
        PowerUpType::WINGMAN,
        PowerUpType::EXTRA_LOOP,
        PowerUpType::FREEZE_BULLETS,
        PowerUpType::EXTRA_LIFE,
    };
    int powIdx = 0;

    for (int r = 0; r < rows; ++r) {
        Formation f;
        f.isRedSquadron = (r % 2 == 0);
        if (f.isRedSquadron) {
            f.powType = powCycle[powIdx % 5];
            ++powIdx;
        }

        for (int c = 0; c < cols; ++c) {
            float tx = 30.f + c * spacingX;
            float ty = -50.f - r * spacingY;
            int   tidx = std::min(r % 5, 4);

            auto e = std::make_unique<Enemy>(
                tx, ty - 200.f,
                EnemyType::SMALL, pat, set[tidx]);
            e->setFormationTarget(tx, ty);

            int enemyIdx = (int)m_enemies.size();
            f.enemyIndices.push_back(enemyIdx);
            m_enemies.push_back(std::move(e));
        }

        m_formations.push_back(std::move(f));
    }

    m_enemyFormation.resize(m_enemies.size(), -1);
    for (int fi = 0; fi < (int)m_formations.size(); ++fi)
        for (int idx : m_formations[fi].enemyIndices)
            m_enemyFormation[idx] = fi;

    // Occasionally add a UFO on higher waves (not part of formations)
    if (waveNumber > 3 && m_ufoTex) {
        float ux = 30.f + (std::rand() % (LOGICAL_W - 60));
        auto u = std::make_unique<Enemy>(
            ux, -60.f, EnemyType::UFO, EnemyPattern::SINE, m_ufoTex);
        m_enemies.push_back(std::move(u));
        m_enemyFormation.push_back(-1);  // UFO has no formation
        ++m_aliveCount;
        ++m_totalSpawned;
    }
}

void EnemyWaveManager::update(float dt) {
    for (auto& e : m_enemies)  e->update(dt);
    for (auto& p : m_powerUps) p->update(dt);

    m_powerUps.erase(
        std::remove_if(m_powerUps.begin(), m_powerUps.end(),
            [](const auto& p){ return !p->isActive(); }),
        m_powerUps.end());
}

void EnemyWaveManager::render(SDL_Renderer* renderer) {
    for (auto& e : m_enemies)  e->render(renderer);
    for (auto& p : m_powerUps) p->render(renderer);
}

void EnemyWaveManager::clear() {
    m_enemies.clear();
    m_powerUps.clear();
    m_formations.clear();
    m_enemyFormation.clear();
    m_aliveCount   = 0;
    m_killCount    = 0;
    m_totalSpawned = 0;
}

bool EnemyWaveManager::isWaveCleared() const {
    for (const auto& e : m_enemies)
        if (e->isActive()) return false;
    return true;
}

void EnemyWaveManager::onEnemyKilled(Enemy* e, int enemyIdx) {
    ++m_killCount;
    --m_aliveCount;

    if (enemyIdx < 0 || enemyIdx >= (int)m_enemyFormation.size()) return;
    int fi = m_enemyFormation[enemyIdx];
    if (fi < 0 || fi >= (int)m_formations.size()) return;

    Formation& f = m_formations[fi];
    if (f.cleared) return;

    for (int i : f.enemyIndices) {
        if (i < (int)m_enemies.size() && m_enemies[i]->isActive())
            return;  // formation not fully cleared yet
    }

    f.cleared = true;
    if (!f.isRedSquadron) return;

    SDL_Texture* tex = getPowTexture(f.powType);
    float px = e->bounds().x + e->bounds().w * 0.5f;
    float py = e->bounds().y;
    m_powerUps.push_back(std::make_unique<PowerUp>(px, py, f.powType, tex));
}

SDL_Texture* EnemyWaveManager::getPowTexture(PowerUpType type) const {
    auto& am = AssetManager::get();
    switch (type) {
    case PowerUpType::DOUBLE_SHOT:    return am.texture("assets/PNG/Power-ups/powerupGreen_bolt.png");
    case PowerUpType::SCREEN_WIPE:    return am.texture("assets/PNG/Power-ups/powerupBlue.png");
    case PowerUpType::WINGMAN:        return am.texture("assets/PNG/Power-ups/powerupYellow.png");
    case PowerUpType::FREEZE_BULLETS: return am.texture("assets/PNG/Power-ups/powerupBlue_star.png");
    case PowerUpType::EXTRA_LOOP:     return am.texture("assets/PNG/Power-ups/powerupGreen.png");
    case PowerUpType::EXTRA_LIFE:     return am.texture("assets/PNG/Power-ups/powerupRed_bolt.png");
    case PowerUpType::SCORE_RED:      return am.texture("assets/PNG/Power-ups/powerupRed.png");
    case PowerUpType::YASHICHI:       return am.texture("assets/PNG/Power-ups/star_gold.png");
    default:                          return nullptr;
    }
}
