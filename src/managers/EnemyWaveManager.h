#pragma once
#include "../entities/Enemy.h"
#include "../entities/PowerUp.h"
#include <vector>
#include <memory>
#include <SDL3/SDL.h>

struct WaveDef {
    int         count;
    EnemyType   type;
    EnemyPattern pattern;
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

    // Called by CollisionManager when an enemy is killed
    void onEnemyKilled(Enemy* e, int waveIdx);

private:
    void buildFormation(const WaveDef& def);

    std::vector<std::unique_ptr<Enemy>>   m_enemies;
    std::vector<std::unique_ptr<PowerUp>> m_powerUps;

    SDL_Texture* m_enemyBlack[5] = {};
    SDL_Texture* m_enemyRed[5]   = {};
    SDL_Texture* m_enemyBlue[5]  = {};
    SDL_Texture* m_enemyGreen[5] = {};
    SDL_Texture* m_ufoTex        = nullptr;

    int   m_waveNumber     = 0;
    float m_spawnTimer     = 0;
    int   m_aliveCount     = 0;
};
