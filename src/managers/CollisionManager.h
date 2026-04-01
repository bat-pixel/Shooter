#pragma once
#include "../entities/Player.h"
#include "../entities/Meteor.h"
#include "BulletManager.h"
#include "EnemyWaveManager.h"
#include <vector>
#include <memory>
#include <functional>

class CollisionManager {
public:
    // Callbacks so Game can react (spawn explosion, add score, etc.)
    using ScoreCb    = std::function<void(int)>;
    using ExplosionCb= std::function<void(float, float)>;

    void setCallbacks(ScoreCb score, ExplosionCb explosion);

    void check(Player& player,
               BulletManager& bullets,
               EnemyWaveManager& waves,
               std::vector<std::unique_ptr<Meteor>>& meteors);

private:
    ScoreCb     m_scoreCb;
    ExplosionCb m_explosionCb;
};
