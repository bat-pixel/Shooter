#include "EnemyWaveManager.h"
#include "../Constants.h"
#include "../AssetManager.h"
#include <cstdlib>
#include <algorithm>

void EnemyWaveManager::loadTextures(SDL_Texture* black[5],
                                    SDL_Texture* red[5],
                                    SDL_Texture* blue[5],
                                    SDL_Texture* green[5],
                                    SDL_Texture* zeroGreen[5],
                                    SDL_Texture* nate[5],
                                    SDL_Texture* oscar[5],
                                    SDL_Texture* val,
                                    SDL_Texture* helen[5],
                                    SDL_Texture* kamikaze,
                                    SDL_Texture* ufo) {
    for (int i = 0; i < 5; ++i) {
        m_enemyBlack[i]     = black[i];
        m_enemyRed[i]       = red[i];
        m_enemyBlue[i]      = blue[i];
        m_enemyGreen[i]     = green[i];
        m_enemyZeroGreen[i] = zeroGreen[i];
        m_enemyNate[i]      = nate[i];
        m_enemyOscar[i]     = oscar[i];
        m_enemyHelen[i]     = helen[i];
    }
    m_enemyVal      = val;
    m_enemyKamikaze = kamikaze;
    m_ufoTex        = ufo;
}

// Per-stage wave scripting. Index = 32 - stageNumber, so stage 32 is [0], stage 1 is [31].
struct StageWaveConfig {
    int          numFormations;
    float        interval;      // seconds between formations
    int          cols;          // Zero fighters per formation
    float        speedMult;     // multiplied onto ENEMY_SPEED_BASE
    int          bomberCount;   // LARGE Betty/Helen formations interspersed (3 cols each)
    int          patternCount;
    EnemyPattern patterns[5];
};

static const StageWaveConfig s_stageCfg[32] = {
    // Stage 32 — Midway 1: gray Zeroes, STRAIGHT + SINE only
    { 8, 4.5f, 5, 1.00f, 0, 2, {EnemyPattern::STRAIGHT, EnemyPattern::SINE}},
    // Stage 31 — Midway 2: DIVE introduced
    { 8, 4.0f, 5, 1.00f, 0, 3, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE}},
    // Stage 30 — Midway 3: ARC + first Betty bombers
    { 9, 3.8f, 6, 1.05f, 1, 4, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC}},
    // Stage 29 — Midway Boss: all patterns
    {10, 3.5f, 6, 1.10f, 1, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 28 — Marshall 1: UFO debut, dual-flank ARC
    { 9, 3.5f, 5, 1.10f, 1, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 27 — Marshall 2: heavy LOOP_DIVE
    {10, 3.2f, 5, 1.15f, 2, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 26 — Marshall 3: wider columns
    {10, 3.0f, 6, 1.20f, 2, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 25 — Marshall Boss
    {11, 2.8f, 6, 1.20f, 2, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 24 — Attu 1: Nell bombers debut
    {10, 3.0f, 6, 1.25f, 2, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 23 — Attu 2: simultaneous flank + center
    {11, 2.8f, 6, 1.30f, 3, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 22 — Attu 3: crossing flankers (both-side ARC)
    {11, 2.6f, 7, 1.35f, 3, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 21 — Attu Boss
    {12, 2.4f, 7, 1.40f, 3, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 20 — Rabaul 1: Double Flankers
    {11, 2.6f, 7, 1.45f, 3, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 19 — Rabaul 2: Bonus Stage (all noFire, dense)
    {14, 2.2f, 8, 1.50f, 2, 2, {EnemyPattern::SINE, EnemyPattern::STRAIGHT}},
    // Stage 18 — Rabaul 3: Double-Decker Bombers
    {12, 2.2f, 7, 1.55f, 4, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 17 — Rabaul Boss
    {13, 2.0f, 7, 1.60f, 4, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 16 — Leyte 1: High-Speed Scouts
    {12, 2.2f, 7, 1.60f, 4, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 15 — Leyte 2: Bonus Stage
    {15, 2.0f, 8, 1.65f, 2, 2, {EnemyPattern::SINE, EnemyPattern::ARC}},
    // Stage 14 — Leyte 3: Zig-Zag formations
    {13, 2.0f, 7, 1.70f, 4, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 13 — Leyte Boss
    {14, 2.0f, 7, 1.75f, 5, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 12 — Saipan 1: Sniper Zeroes
    {13, 2.0f, 7, 1.75f, 5, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 11 — Saipan 2: Bonus Stage
    {16, 2.0f, 8, 1.80f, 2, 3, {EnemyPattern::SINE, EnemyPattern::ARC, EnemyPattern::STRAIGHT}},
    // Stage 10 — Saipan 3: Sniper Zeroes continue
    {14, 2.0f, 7, 1.85f, 5, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 9 — Saipan Boss
    {15, 2.0f, 7, 1.90f, 5, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 8 — Iwojima 1: Chaos — sniper + kamikaze + all types
    {14, 2.0f, 7, 1.90f, 5, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 7 — Iwojima 2: Final Bonus Stage
    {18, 1.8f, 8, 1.95f, 2, 3, {EnemyPattern::SINE, EnemyPattern::ARC, EnemyPattern::STRAIGHT}},
    // Stage 6 — Iwojima 3: Red Chain (15-plane chain + snipers)
    {15, 2.0f, 7, 2.00f, 6, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 5 — Iwojima Boss
    {16, 2.0f, 7, 2.00f, 6, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 4 — Okinawa 1: Maximum chaos, all enemy types, sniper
    {15, 2.0f, 7, 2.00f, 6, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 3 — Okinawa 2
    {16, 2.0f, 7, 2.10f, 7, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 2 — Okinawa 3
    {16, 2.0f, 7, 2.20f, 7, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
    // Stage 1 — Okinawa Final Boss
    {16, 2.0f, 7, 2.30f, 7, 5, {EnemyPattern::STRAIGHT, EnemyPattern::SINE, EnemyPattern::DIVE, EnemyPattern::ARC, EnemyPattern::LOOP_DIVE}},
};

// Bonus stages: no enemy fire, dense formations for point farming
static bool isBonusStage(int stageNumber) {
    return stageNumber == 19 || stageNumber == 15 ||
           stageNumber == 11 || stageNumber == 7;
}

void EnemyWaveManager::startWave(int stageNumber) {
    m_stageNumber = stageNumber;
    m_enemies.clear();
    m_formations.clear();
    m_enemyFormation.clear();
    m_powerUps.clear();
    m_pendingQueue.clear();
    m_pendingIdx   = 0;
    m_waveTimer    = 0;
    m_aliveCount   = 0;
    m_totalSpawned = 0;

    int cfgIdx = std::clamp(32 - stageNumber, 0, 31);
    const StageWaveConfig& cfg = s_stageCfg[cfgIdx];

    bool bonus = isBonusStage(stageNumber);

    // Spread bomber formations evenly through the wave
    // Bonus stages: skip bombers (just fighters for a clean shooting gallery)
    std::vector<bool> isBomber(cfg.numFormations, false);
    if (cfg.bomberCount > 0 && !bonus) {
        float step = (float)cfg.numFormations / (cfg.bomberCount + 1);
        for (int b = 0; b < cfg.bomberCount; ++b) {
            int slot = std::min((int)((b + 1) * step), cfg.numFormations - 1);
            isBomber[slot] = true;
        }
    }

    static const PowerUpType powCycle[] = {
        PowerUpType::DOUBLE_SHOT,
        PowerUpType::WINGMAN,
        PowerUpType::EXTRA_LOOP,
        PowerUpType::FREEZE_BULLETS,
        PowerUpType::EXTRA_LIFE,
    };

    // Fighter color sets: black/red/blue (all), +ZeroGreen (stage≤24), +Nate (stage≤8), +Oscar (stage≤4)
    SDL_Texture** zeroSets[6]   = { m_enemyBlack, m_enemyRed, m_enemyBlue, m_enemyZeroGreen, m_enemyNate, m_enemyOscar };
    float         zeroAngles[6] = { 0.f,          0.f,        0.f,         0.f,              0.f,         180.f        };
    int numColorSets = (stageNumber <= 4) ? 6 : (stageNumber <= 8) ? 5 : (stageNumber <= 24) ? 4 : 3;
    int zeroFormIdx = 0;

    // Late stages use Helen heavy bomber instead of Betty
    SDL_Texture** bomberSet = (stageNumber <= 16 && m_enemyHelen[0]) ? m_enemyHelen : m_enemyGreen;

    // Sniper active from Saipan onward (stages ≤12) — not on boss stages
    bool sniperActive = (stageNumber <= 12) && !bonus;

    for (int f = 0; f < cfg.numFormations; ++f) {
        PendingFormation pf;
        pf.delay     = f * cfg.interval;
        pf.speedMult = cfg.speedMult;
        pf.noFire    = bonus;

        if (isBomber[f]) {
            pf.cols          = 3;
            pf.type          = EnemyType::LARGE;
            pf.pattern       = EnemyPattern::STRAIGHT;
            pf.isRedSquadron = false;
            pf.powType       = PowerUpType::EXTRA_LIFE;
            pf.baseAngle     = (bomberSet == m_enemyHelen) ? 180.f : 0.f;
            for (int i = 0; i < 5; ++i) pf.texSet[i] = bomberSet[i];
        } else {
            pf.cols          = cfg.cols;
            pf.type          = EnemyType::SMALL;
            pf.pattern       = cfg.patterns[zeroFormIdx % cfg.patternCount];
            pf.isRedSquadron = (zeroFormIdx % 2 == 0);
            pf.powType       = powCycle[zeroFormIdx % 5];
            int setIdx       = zeroFormIdx % numColorSets;
            pf.baseAngle     = zeroAngles[setIdx];
            pf.isSniper      = sniperActive;
            SDL_Texture** set = zeroSets[setIdx];
            for (int i = 0; i < 5; ++i) pf.texSet[i] = set[i];
            ++zeroFormIdx;
        }

        m_pendingQueue.push_back(pf);
    }

    // ---- Stage-specific special mechanics ----

    float waveLen = (cfg.numFormations - 1) * cfg.interval;

    // Stage 20 — Double Flankers: simultaneous ARC pairs from both screen edges
    if (stageNumber == 20) {
        for (int k = 0; k < 3; ++k) {
            float t = waveLen * 0.25f + k * 2.2f;
            PendingFormation lf, rf;
            lf.delay = t; lf.cols = 5; lf.type = EnemyType::SMALL;
            lf.pattern = EnemyPattern::ARC;
            lf.isRedSquadron = (k == 1); lf.powType = PowerUpType::EXTRA_LOOP;
            lf.speedMult = cfg.speedMult; lf.forceArcSide = 0;  // left spawn
            for (int i = 0; i < 5; ++i) lf.texSet[i] = m_enemyZeroGreen[i];
            rf = lf; rf.forceArcSide = 1;  // right spawn, same delay
            m_pendingQueue.push_back(lf);
            m_pendingQueue.push_back(rf);
        }
    }

    // Stage 18 — Double-Decker Bombers: extra wide 6-bomber formation
    if (stageNumber == 18) {
        PendingFormation dd;
        dd.delay = waveLen * 0.55f;
        dd.cols = 6; dd.type = EnemyType::LARGE;
        dd.pattern = EnemyPattern::STRAIGHT;
        dd.isRedSquadron = false; dd.powType = PowerUpType::EXTRA_LIFE;
        dd.speedMult = cfg.speedMult;
        dd.baseAngle = (bomberSet == m_enemyHelen) ? 180.f : 0.f;
        for (int i = 0; i < 5; ++i) dd.texSet[i] = bomberSet[i];
        m_pendingQueue.push_back(dd);
    }

    // Stage 16 — High-Speed Scouts: fast black fighters that cross without firing
    if (stageNumber == 16) {
        for (int k = 0; k < 2; ++k) {
            PendingFormation sf;
            sf.delay = waveLen * 0.45f + k * 2.5f;
            sf.cols = 4; sf.type = EnemyType::SMALL;
            sf.pattern = EnemyPattern::SINE;
            sf.isRedSquadron = false; sf.powType = PowerUpType::SCORE_RED;
            sf.speedMult = cfg.speedMult * 2.4f;
            sf.noFire = true;
            sf.baseAngle = 0.f;
            for (int i = 0; i < 5; ++i) sf.texSet[i] = m_enemyBlack[i];
            m_pendingQueue.push_back(sf);
        }
    }

    // Stage 14 — Zig-Zag: change fighter patterns to ZIGZAG
    if (stageNumber == 14) {
        for (auto& pf : m_pendingQueue) {
            if (pf.type == EnemyType::SMALL &&
                (pf.pattern == EnemyPattern::SINE || pf.pattern == EnemyPattern::STRAIGHT)) {
                pf.pattern = EnemyPattern::ZIGZAG;
            }
        }
    }

    // Stage 6 — Red Chain: 5 rapid-fire red sniper formations (= 15 planes)
    if (stageNumber == 6) {
        float chainStart = waveLen * 0.65f;
        for (int k = 0; k < 5; ++k) {
            PendingFormation cf;
            cf.delay = chainStart + k * 0.6f;
            cf.cols = 3; cf.type = EnemyType::SMALL;
            cf.pattern = EnemyPattern::SINE;
            cf.isRedSquadron = true;
            cf.powType = (k == 4) ? PowerUpType::YASHICHI : PowerUpType::SCORE_RED;
            cf.speedMult = cfg.speedMult;
            cf.isSniper = true;
            for (int i = 0; i < 5; ++i) cf.texSet[i] = m_enemyRed[i];
            m_pendingQueue.push_back(cf);
        }
    }

    // Kamikaze dive-bomber squads unlock at Saipan (stage 12) and scale up — skip bonus stages
    if (stageNumber <= 12 && m_enemyKamikaze && !bonus) {
        int kamikazeCount = 1 + (12 - stageNumber) / 4;
        kamikazeCount = std::min(kamikazeCount, 3);
        for (int k = 0; k < kamikazeCount; ++k) {
            PendingFormation kf;
            kf.delay         = waveLen * (k + 1) / (kamikazeCount + 1);
            kf.cols          = (stageNumber <= 5) ? 5 : 3;
            kf.type          = EnemyType::MEDIUM;
            kf.pattern       = EnemyPattern::LOOP_DIVE;
            kf.isRedSquadron = false;
            kf.powType       = PowerUpType::EXTRA_LOOP;
            kf.speedMult     = cfg.speedMult;
            for (int i = 0; i < 5; ++i) kf.texSet[i] = m_enemyKamikaze;
            m_pendingQueue.push_back(kf);
        }
    }

    // Val dive-bomber squad: mid-game exclusive (Rabaul through Saipan) — skip bonus stages
    if (stageNumber <= 20 && stageNumber > 8 && m_enemyVal && !bonus) {
        PendingFormation vf;
        vf.delay         = waveLen * 0.4f;
        vf.cols          = 3;
        vf.type          = EnemyType::MEDIUM;
        vf.pattern       = EnemyPattern::DIVE;
        vf.isRedSquadron = true;
        vf.powType       = PowerUpType::SCORE_RED;
        vf.speedMult     = cfg.speedMult;
        vf.baseAngle     = 180.f;  // south-facing sprite
        for (int i = 0; i < 5; ++i) vf.texSet[i] = m_enemyVal;
        m_pendingQueue.push_back(vf);
    }

    // UFO mid-wave from Marshall campaign onward (including bonus stages — bonus target!)
    if (stageNumber <= 28 && m_ufoTex) {
        PendingFormation uf;
        uf.delay         = (cfg.numFormations / 2) * cfg.interval;
        uf.cols          = 1;
        uf.isRedSquadron = false;
        uf.powType       = PowerUpType::SCORE_RED;
        uf.pattern       = EnemyPattern::SINE;
        uf.type          = EnemyType::UFO;
        uf.speedMult     = cfg.speedMult;
        uf.noFire        = bonus;
        for (int i = 0; i < 5; ++i) uf.texSet[i] = m_ufoTex;
        m_pendingQueue.push_back(uf);
    }

    // Sort so drip-spawner sees formations in time order
    std::stable_sort(m_pendingQueue.begin(), m_pendingQueue.end(),
        [](const PendingFormation& a, const PendingFormation& b){
            return a.delay < b.delay;
        });
}

void EnemyWaveManager::spawnFormation(const PendingFormation& pf) {
    int formIdx = (int)m_formations.size();
    Formation f;
    f.isRedSquadron = pf.isRedSquadron;
    f.powType       = pf.powType;

    auto applyBase = [&](Enemy* e) {
        if (pf.baseAngle != 0.f) e->setBaseAngle(pf.baseAngle);
        if (pf.isSniper)         e->setSniper(true);
        if (pf.noFire)           e->setNoFire(true);
    };

    if (pf.type == EnemyType::UFO) {
        float ux = 30.f + (std::rand() % (LOGICAL_W - 60));
        auto u = std::make_unique<Enemy>(ux, -60.f, EnemyType::UFO, EnemyPattern::SINE,
                                         pf.texSet[0], nullptr, pf.speedMult);
        applyBase(u.get());
        int idx = (int)m_enemies.size();
        f.enemyIndices.push_back(idx);
        m_enemies.push_back(std::move(u));
        m_enemyFormation.push_back(formIdx);
        ++m_aliveCount;
        ++m_totalSpawned;
    } else if (pf.pattern == EnemyPattern::ARC) {
        // Determine spawn side: forced or auto-alternating
        bool fromLeft;
        if (pf.forceArcSide >= 0)
            fromLeft = (pf.forceArcSide == 0);
        else
            fromLeft = (formIdx % 2 == 0);

        float edgeX = fromLeft ? 5.f : LOGICAL_W - 35.f;
        float xStep = fromLeft ? 18.f : -18.f;
        for (int c = 0; c < pf.cols; ++c) {
            float tx = edgeX + c * xStep;
            float ty = -50.f - c * 35.f;
            auto e = std::make_unique<Enemy>(tx, ty, pf.type, pf.pattern, pf.texSet[c % 5],
                                             nullptr, pf.speedMult);
            applyBase(e.get());
            int idx = (int)m_enemies.size();
            f.enemyIndices.push_back(idx);
            m_enemies.push_back(std::move(e));
            m_enemyFormation.push_back(formIdx);
            ++m_aliveCount;
            ++m_totalSpawned;
        }
    } else if (pf.pattern == EnemyPattern::LOOP_DIVE) {
        float spacingX = (LOGICAL_W - 60.f) / pf.cols;
        for (int c = 0; c < pf.cols; ++c) {
            float tx = 30.f + c * spacingX;
            float ty = -50.f - c * 20.f;
            auto e = std::make_unique<Enemy>(tx, ty, pf.type, pf.pattern, pf.texSet[c % 5],
                                             nullptr, pf.speedMult);
            applyBase(e.get());
            int idx = (int)m_enemies.size();
            f.enemyIndices.push_back(idx);
            m_enemies.push_back(std::move(e));
            m_enemyFormation.push_back(formIdx);
            ++m_aliveCount;
            ++m_totalSpawned;
        }
    } else {
        float spacingX = (LOGICAL_W - 60.f) / std::max(pf.cols, 1);
        for (int c = 0; c < pf.cols; ++c) {
            float tx = 30.f + c * spacingX;
            float ty = -50.f;
            auto e = std::make_unique<Enemy>(
                tx, ty - 200.f, pf.type, pf.pattern, pf.texSet[c % 5],
                nullptr, pf.speedMult);
            applyBase(e.get());
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
