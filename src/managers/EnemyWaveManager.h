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

class EnemyWaveManager {
public:
    EnemyWaveManager() = default;

    void loadTextures(SDL_Texture* enemyBlack[5],
                      SDL_Texture* enemyRed[5],
                      SDL_Texture* enemyBlue[5],
                      SDL_Texture* enemyGreen[5],
                      SDL_Texture* ufoTex);

    void startWave(int waveNumber);
    void update(float dt);
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
    SDL_Texture* getPowTexture(PowerUpType type) const;

    std::vector<std::unique_ptr<Enemy>>   m_enemies;
    std::vector<std::unique_ptr<PowerUp>> m_powerUps;
    std::vector<Formation>                m_formations;
    std::vector<int>                      m_enemyFormation;

    SDL_Texture* m_enemyBlack[5] = {};
    SDL_Texture* m_enemyRed[5]   = {};
    SDL_Texture* m_enemyBlue[5]  = {};
    SDL_Texture* m_enemyGreen[5] = {};
    SDL_Texture* m_ufoTex        = nullptr;

    int m_waveNumber   = 0;
    int m_aliveCount   = 0;
    int m_killCount    = 0;
    int m_totalSpawned = 0;
};
