#pragma once
#include "../entities/Enemy.h"
#include "../entities/PowerUp.h"
#include <vector>
#include <memory>
#include <SDL3/SDL.h>

struct Formation {
    std::vector<int> enemyIndices;
    bool             isRedSquadron = false;
    bool             cleared       = false;
    PowerUpType      powType       = PowerUpType::DOUBLE_SHOT;
};

struct PendingFormation {
    float        delay;          // seconds from wave start
    int          cols;
    EnemyPattern pattern;
    EnemyType    type;
    SDL_Texture* texSet[5];
    bool         isRedSquadron;
    PowerUpType  powType;
    float        speedMult = 1.f;
};

class EnemyWaveManager {
public:
    EnemyWaveManager() = default;

    void loadTextures(SDL_Texture* enemyBlack[5],
                      SDL_Texture* enemyRed[5],
                      SDL_Texture* enemyBlue[5],
                      SDL_Texture* enemyGreen[5],
                      SDL_Texture* enemyZeroGreen[5],
                      SDL_Texture* enemyNate[5],
                      SDL_Texture* enemyKamikaze,
                      SDL_Texture* ufoTex);

    void startWave(int stageNumber);
    void update(float dt, float playerX = 180.f, float playerY = 400.f);
    void render(SDL_Renderer* renderer);
    void clear();

    bool isWaveCleared() const;

    std::vector<std::unique_ptr<Enemy>>&   enemies()  { return m_enemies;  }
    std::vector<std::unique_ptr<PowerUp>>& powerUps() { return m_powerUps; }

    void onEnemyKilled(Enemy* e, int enemyIdx);

    int killCount()    const { return m_killCount;    }
    int totalSpawned() const { return m_totalSpawned; }
    void resetKillStats()    { m_killCount = 0; m_totalSpawned = 0; }

private:
    void spawnFormation(const PendingFormation& pf);
    SDL_Texture* getPowTexture(PowerUpType type) const;

    std::vector<std::unique_ptr<Enemy>>   m_enemies;
    std::vector<std::unique_ptr<PowerUp>> m_powerUps;
    std::vector<Formation>                m_formations;
    std::vector<int>                      m_enemyFormation;

    std::vector<PendingFormation>         m_pendingQueue;
    int                                   m_pendingIdx  = 0;
    float                                 m_waveTimer   = 0;

    SDL_Texture* m_enemyBlack[5]     = {};
    SDL_Texture* m_enemyRed[5]       = {};
    SDL_Texture* m_enemyBlue[5]      = {};
    SDL_Texture* m_enemyGreen[5]     = {};
    SDL_Texture* m_enemyZeroGreen[5] = {};
    SDL_Texture* m_enemyNate[5]      = {};
    SDL_Texture* m_enemyKamikaze     = nullptr;
    SDL_Texture* m_ufoTex            = nullptr;

    int m_stageNumber  = 32;
    int m_aliveCount   = 0;
    int m_killCount    = 0;
    int m_totalSpawned = 0;
};
