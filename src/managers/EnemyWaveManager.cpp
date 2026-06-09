#include "EnemyWaveManager.h"
#include "../Constants.h"
#include "../AssetManager.h"
#include <cstdlib>
#include <algorithm>

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
    m_pendingQueue.clear();
    m_pendingIdx   = 0;
    m_waveTimer    = 0;
    m_aliveCount   = 0;
    m_totalSpawned = 0;

    // Number of formations scales with wave; earlier waves get enough to feel substantial
    int numFormations = std::min(4 + waveNumber / 2, 14);
    // Interval between formations shortens slightly as difficulty increases
    float interval = std::max(3.5f - waveNumber * 0.06f, 2.0f);

    static const PowerUpType powCycle[] = {
        PowerUpType::DOUBLE_SHOT,
        PowerUpType::WINGMAN,
        PowerUpType::EXTRA_LOOP,
        PowerUpType::FREEZE_BULLETS,
        PowerUpType::EXTRA_LIFE,
    };

    SDL_Texture** sets[4] = { m_enemyBlack, m_enemyRed, m_enemyBlue, m_enemyGreen };

    for (int f = 0; f < numFormations; ++f) {
        PendingFormation pf;
        pf.delay        = f * interval;
        pf.cols         = 5;
        pf.isRedSquadron= (f % 2 == 0);
        pf.powType      = powCycle[f % 5];
        pf.type         = EnemyType::SMALL;

        switch (f % 5) {
        case 0: pf.pattern = EnemyPattern::STRAIGHT;  break;
        case 1: pf.pattern = EnemyPattern::SINE;      break;
        case 2: pf.pattern = EnemyPattern::DIVE;      break;
        case 3: pf.pattern = EnemyPattern::ARC;       break;
        case 4: pf.pattern = EnemyPattern::LOOP_DIVE; break;
        }

        SDL_Texture** set = sets[f % 4];
        for (int i = 0; i < 5; ++i) pf.texSet[i] = set[i];

        m_pendingQueue.push_back(pf);
    }

    // Add a UFO mid-wave on higher difficulties
    if (waveNumber > 3 && m_ufoTex) {
        PendingFormation uf;
        uf.delay         = (numFormations / 2) * interval;
        uf.cols          = 1;
        uf.isRedSquadron = false;
        uf.powType       = PowerUpType::SCORE_RED;
        uf.pattern       = EnemyPattern::SINE;
        uf.type          = EnemyType::UFO;
        for (int i = 0; i < 5; ++i) uf.texSet[i] = m_ufoTex;
        m_pendingQueue.push_back(uf);
        // Sort so UFO fires at the right time
        std::stable_sort(m_pendingQueue.begin(), m_pendingQueue.end(),
            [](const PendingFormation& a, const PendingFormation& b){
                return a.delay < b.delay;
            });
    }
}

void EnemyWaveManager::spawnFormation(const PendingFormation& pf) {
    int formIdx = (int)m_formations.size();
    Formation f;
    f.isRedSquadron = pf.isRedSquadron;
    f.powType       = pf.powType;

    if (pf.type == EnemyType::UFO) {
        float ux = 30.f + (std::rand() % (LOGICAL_W - 60));
        auto u = std::make_unique<Enemy>(ux, -60.f, EnemyType::UFO, EnemyPattern::SINE, pf.texSet[0]);
        int idx = (int)m_enemies.size();
        f.enemyIndices.push_back(idx);
        m_enemies.push_back(std::move(u));
        m_enemyFormation.push_back(formIdx);
        ++m_aliveCount;
        ++m_totalSpawned;
    } else if (pf.pattern == EnemyPattern::ARC) {
        // Spawn a staggered column from left or right edge
        bool fromLeft = (formIdx % 2 == 0);
        float edgeX = fromLeft ? 5.f : LOGICAL_W - 35.f;
        float xStep = fromLeft ? 18.f : -18.f;
        for (int c = 0; c < pf.cols; ++c) {
            float tx = edgeX + c * xStep;
            float ty = -50.f - c * 35.f;  // stagger entry so they arrive sequentially
            auto e = std::make_unique<Enemy>(tx, ty, pf.type, pf.pattern, pf.texSet[c % 5]);
            int idx = (int)m_enemies.size();
            f.enemyIndices.push_back(idx);
            m_enemies.push_back(std::move(e));
            m_enemyFormation.push_back(formIdx);
            ++m_aliveCount;
            ++m_totalSpawned;
        }
    } else if (pf.pattern == EnemyPattern::LOOP_DIVE) {
        // Spawn in standard row; no formation approach, loop starts immediately on-screen
        float spacingX = (LOGICAL_W - 60.f) / pf.cols;
        for (int c = 0; c < pf.cols; ++c) {
            float tx = 30.f + c * spacingX;
            float ty = -50.f - c * 20.f;  // slight stagger for visual interest
            auto e = std::make_unique<Enemy>(tx, ty, pf.type, pf.pattern, pf.texSet[c % 5]);
            int idx = (int)m_enemies.size();
            f.enemyIndices.push_back(idx);
            m_enemies.push_back(std::move(e));
            m_enemyFormation.push_back(formIdx);
            ++m_aliveCount;
            ++m_totalSpawned;
        }
    } else {
        float spacingX = (LOGICAL_W - 60.f) / pf.cols;
        for (int c = 0; c < pf.cols; ++c) {
            float tx = 30.f + c * spacingX;
            float ty = -50.f;
            auto e = std::make_unique<Enemy>(
                tx, ty - 200.f, pf.type, pf.pattern, pf.texSet[c % 5]);
            e->setFormationTarget(tx, ty);
            int idx = (int)m_enemies.size();
            f.enemyIndices.push_back(idx);
            m_enemies.push_back(std::move(e));
            m_enemyFormation.push_back(formIdx);
            ++m_aliveCount;
            ++m_totalSpawned;
        }
    }

    m_formations.push_back(std::move(f));
}

void EnemyWaveManager::update(float dt, float playerX, float playerY) {
    m_waveTimer += dt;

    // Drip-spawn formations as their delay elapses
    while (m_pendingIdx < (int)m_pendingQueue.size() &&
           m_waveTimer >= m_pendingQueue[m_pendingIdx].delay) {
        spawnFormation(m_pendingQueue[m_pendingIdx]);
        ++m_pendingIdx;
    }

    for (auto& e : m_enemies) {
        e->setPlayerTarget(playerX, playerY);
        e->update(dt);
    }
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
    m_pendingQueue.clear();
    m_pendingIdx   = 0;
    m_waveTimer    = 0;
    m_aliveCount   = 0;
    m_killCount    = 0;
    m_totalSpawned = 0;
}

bool EnemyWaveManager::isWaveCleared() const {
    if (m_pendingIdx < (int)m_pendingQueue.size()) return false;
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
            return;
    }

    f.cleared = true;
    if (!f.isRedSquadron) return;

    SDL_Texture* tex = getPowTexture(f.powType);
    float px = e->bounds().x + e->bounds().w * 0.5f;
    float py = e->bounds().y;
    m_powerUps.push_back(std::make_unique<PowerUp>(px, py, f.powType, tex));
}

SDL_Texture* EnemyWaveManager::getPowTexture(PowerUpType /*type*/) const {
    return AssetManager::get().texture("assets/PNG/Power-ups/powBox.png");
}
