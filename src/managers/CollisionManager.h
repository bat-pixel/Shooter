#pragma once
#include "../entities/Player.h"
#include "BulletManager.h"
#include "EnemyWaveManager.h"
#include <functional>

class CollisionManager {
public:
    using ScoreCb    = std::function<void(int)>;
    using ExplosionCb= std::function<void(float, float)>;

    void setCallbacks(ScoreCb score, ExplosionCb explosion);

    void check(Player& player,
               BulletManager& bullets,
               EnemyWaveManager& waves);

private:
    ScoreCb     m_scoreCb;
    ExplosionCb m_explosionCb;
};
