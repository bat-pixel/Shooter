#include "Game.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "InputManager.h"
#include "StageManager.h"
#include "Constants.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <array>

// -----------------------------------------------------------------------
bool Game::init() {
    std::srand((unsigned)std::time(nullptr));

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return false;
    }

    m_window = SDL_CreateWindow("1942",
                                 360, 640,
                                 SDL_WINDOW_RESIZABLE);
    if (!m_window) { SDL_Log("CreateWindow: %s", SDL_GetError()); return false; }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) { SDL_Log("CreateRenderer: %s", SDL_GetError()); return false; }

    SDL_SetRenderLogicalPresentation(m_renderer,
                                     LOGICAL_W, LOGICAL_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    std::string base = SDL_GetBasePath();
    AssetManager::get().init(m_renderer, base);
    AudioManager::get().init(base);

    auto& am = AssetManager::get();

    // Campaign-specific background tiles (fall back to ocean if not yet available)
    SDL_Texture* oceanTex = am.texture("assets/PNG/Backgrounds/bg_ocean.png");
    m_campaignBg[0] = am.texture("assets/PNG/Backgrounds/bg_midway.png");
    m_campaignBg[1] = am.texture("assets/PNG/Backgrounds/bg_marshall.png");
    m_campaignBg[2] = am.texture("assets/PNG/Backgrounds/bg_attu.png");
    m_campaignBg[3] = am.texture("assets/PNG/Backgrounds/bg_rabaul.png");
    m_campaignBg[4] = am.texture("assets/PNG/Backgrounds/bg_leyte.png");
    m_campaignBg[5] = am.texture("assets/PNG/Backgrounds/bg_saipan.png");
    m_campaignBg[6] = am.texture("assets/PNG/Backgrounds/bg_iwojima.png");
    m_campaignBg[7] = am.texture("assets/PNG/Backgrounds/bg_tokyo.png");
    // Use ocean as fallback for any missing campaign backgrounds
    for (auto& t : m_campaignBg) if (!t) t = oceanTex;

    m_menuBg = am.texture("assets/intro_screen.png");
    if (!m_menuBg) m_menuBg = oceanTex;
    m_background = std::make_unique<Background>(
        std::vector<SDL_Texture*>{m_campaignBg[0]}, 80.f);

    // Atmospheric cloud sprites
    m_cloudTex[0] = am.texture("assets/PNG/Effects/cloud_light.png");
    m_cloudTex[1] = am.texture("assets/PNG/Effects/cloud_dark.png");

    // Explosion sprite (single frame, scaled to 64px at render)
    SDL_Texture* explosionTex = am.texture("assets/PNG/Effects/explosion.png");
    if (explosionTex) m_explosionFrames.push_back(explosionTex);

    // Player
    SDL_Texture* shipTex    = am.texture("assets/PNG/playerShip_p38.png");
    SDL_Texture* wingmanTex = am.texture("assets/PNG/playerWingman.png");
    std::array<SDL_Texture*, 3> damageTex = {nullptr, nullptr, nullptr};
    m_player = std::make_unique<Player>(
        LOGICAL_W * 0.5f - 24, LOGICAL_H - 120.f,
        shipTex, damageTex, wingmanTex);

    // Enemy textures — Zero variants + bombers + new sprites
    SDL_Texture* zeroTex      = am.texture("assets/PNG/Enemies/enemyZero.png");
    SDL_Texture* zeroRed      = am.texture("assets/PNG/Enemies/enemyZero_red.png");
    SDL_Texture* zeroBlue     = am.texture("assets/PNG/Enemies/enemyZero_blue.png");
    SDL_Texture* zeroGreenTex = am.texture("assets/PNG/Enemies/enemyZero_green.png");
    SDL_Texture* nateTex      = am.texture("assets/PNG/Enemies/enemyNate.png");
    SDL_Texture* oscarTex     = am.texture("assets/PNG/Enemies/enemyOscar.png");
    SDL_Texture* valTex       = am.texture("assets/PNG/Enemies/enemyVal.png");
    SDL_Texture* betty        = am.texture("assets/PNG/Enemies/enemyBetty.png");
    SDL_Texture* nell         = am.texture("assets/PNG/Enemies/enemyNell.png");
    SDL_Texture* helenTex     = am.texture("assets/PNG/Enemies/enemyHelen.png");
    SDL_Texture* ufoTex       = am.texture("assets/PNG/Enemies/enemyUFO.png");
    m_enemyKamikazeTex        = am.texture("assets/PNG/Enemies/enemyKamikaze.png");
    m_bossKagaTex             = am.texture("assets/PNG/Enemies/bossKaga.png");
    m_bossCruiserTex          = am.texture("assets/PNG/Enemies/bossCruiser.png");
    m_bossYamatoTex           = am.texture("assets/PNG/Enemies/bossYamato.png");

    SDL_Texture* black[5], *red[5], *blue[5], *green[5], *zeroGreen[5], *nate[5], *oscar[5], *helen[5];
    for (int i = 0; i < 5; ++i) {
        black[i]     = zeroTex      ? zeroTex      : nullptr;
        red[i]       = zeroRed      ? zeroRed      : zeroTex;
        blue[i]      = zeroBlue     ? zeroBlue     : zeroTex;
        green[i]     = betty        ? betty        : nell;
        zeroGreen[i] = zeroGreenTex ? zeroGreenTex : zeroTex;
        nate[i]      = nateTex      ? nateTex      : zeroTex;
        oscar[i]     = oscarTex     ? oscarTex     : zeroTex;
        helen[i]     = helenTex     ? helenTex     : (betty ? betty : nell);
    }
    m_waves.loadTextures(black, red, blue, green, zeroGreen, nate, oscar, valTex, helen,
                         m_enemyKamikazeTex, ufoTex ? ufoTex : nell);

    // Bullet sprites
    m_playerBulletTex = am.texture("assets/PNG/Lasers/playerBullet.png");
    m_enemyBulletTex  = am.texture("assets/PNG/Lasers/enemyBullet.png");

    // Boss
    m_bossTex       = am.texture("assets/PNG/Enemies/bossAyako.png");
    m_carrierDeckTex = am.texture("assets/PNG/Terrain/carrier_landing.png");

    // Terrain objects — islands categorized by environment type
    m_terrainBigTex     = am.texture("assets/PNG/Terrain/big_island.png");
    m_terrainCarrierTex = am.texture("assets/PNG/Terrain/carrier_ship.png");

    auto loadIsland = [&](const char* path, std::vector<SDL_Texture*>& group) {
        if (SDL_Texture* t = am.texture(path)) { group.push_back(t); m_terrainSmallTextures.push_back(t); }
    };
    // Tropical palm islands
    loadIsland("assets/PNG/Terrain/island_palm_round.png",      m_islandsPalm);
    loadIsland("assets/PNG/Terrain/island_palm_irregular.png",  m_islandsPalm);
    loadIsland("assets/PNG/Terrain/island_palm_large.png",      m_islandsPalm);
    loadIsland("assets/PNG/Terrain/island_palm_oval.png",       m_islandsPalm);
    // Coral atolls
    loadIsland("assets/PNG/Terrain/island_atoll_ring.png",      m_islandsAtoll);
    loadIsland("assets/PNG/Terrain/island_atoll_small.png",     m_islandsAtoll);
    loadIsland("assets/PNG/Terrain/island_atoll_sandy.png",     m_islandsAtoll);
    loadIsland("assets/PNG/Terrain/island_atoll_wreath.png",    m_islandsAtoll);
    // Volcanic islands
    loadIsland("assets/PNG/Terrain/island_volcano_mountain.png",m_islandsVolcano);
    loadIsland("assets/PNG/Terrain/island_volcano_erupting.png",m_islandsVolcano);
    loadIsland("assets/PNG/Terrain/island_volcano_crater.png",  m_islandsVolcano);
    // Forest / jungle islands
    loadIsland("assets/PNG/Terrain/island_forest_round.png",    m_islandsForest);
    loadIsland("assets/PNG/Terrain/island_forest_dark.png",     m_islandsForest);
    loadIsland("assets/PNG/Terrain/island_forest_rocky.png",    m_islandsForest);
    loadIsland("assets/PNG/Terrain/island_forest_dome.png",     m_islandsForest);
    // Fallback: generic small island (added to master list only)
    if (SDL_Texture* t = am.texture("assets/PNG/Terrain/small_island.png"))
        m_terrainSmallTextures.push_back(t);

    // HUD — use P-38 sprite as life icon
    TTF_Font* font    = am.font("assets/Bonus/kenvector_future.ttf", 16);
    SDL_Texture* lifeIcon = wingmanTex ? wingmanTex : shipTex;
    m_hud = std::make_unique<HUD>(font, lifeIcon);

    // Collision callbacks
    m_collision.setCallbacks(
        [this](int pts){ m_score += pts; },
        [this](float x, float y){ spawnExplosion(x, y); }
    );

    // m_menuBg already set above

    AudioManager::get().playMusic("assets/music/Skyfire Patrol intro.mp3");

    m_running = true;
    return true;
}

// -----------------------------------------------------------------------
void Game::run() {
    Uint64 lastTicks = SDL_GetTicks();
    while (m_running) {
        Uint64 now = SDL_GetTicks();
        float dt = (now - lastTicks) / 1000.0f;
        lastTicks = now;
        dt = (dt > MAX_DT) ? MAX_DT : dt;

        handleEvents();
        update(dt);
        render();
    }
}

// -----------------------------------------------------------------------
void Game::handleEvents() {
    auto& input = InputManager::get();
    input.endFrame();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) { m_running = false; return; }
        if (e.type == SDL_EVENT_MOUSE_MOTION) {
            m_mouseIdleTimer = 0.f;
            SDL_ShowCursor();
        }
        input.update(e);
    }

    // T key: in menu → open level select; in play → toggle god mode
    if (input.isPressed(Action::TRAINING)) {
        if (m_state == GameState::MENU)
            m_state = GameState::LEVEL_SELECT;
        else if (m_player)
            m_player->setGodMode(!m_player->godMode());
    }

    if (m_player && m_player->godMode()) {
        static constexpr PowerUpType kSlots[] = {
            PowerUpType::DOUBLE_SHOT, PowerUpType::SCREEN_WIPE,
            PowerUpType::WINGMAN,     PowerUpType::FREEZE_BULLETS,
            PowerUpType::EXTRA_LOOP,  PowerUpType::EXTRA_LIFE,
            PowerUpType::SCORE_RED,   PowerUpType::YASHICHI,
        };
        int slot = input.pressedPowerUpSlot();
        if (slot >= 1 && slot <= 8) {
            PowerUpType pt = kSlots[slot - 1];
            if      (pt == PowerUpType::SCORE_RED)  m_score += SCORE_POW_RED;
            else if (pt == PowerUpType::YASHICHI)   m_score += SCORE_YASHICHI;
            else m_player->applyPowerUp(pt);
        }
    }

    if (input.isPressed(Action::PAUSE)) {
        if (m_state == GameState::PLAYING)
            m_state = GameState::PAUSED;
        else if (m_state == GameState::PAUSED)
            m_state = GameState::PLAYING;
    }

    // Level select navigation
    if (m_state == GameState::LEVEL_SELECT) {
        if (input.isPressed(Action::MOVE_LEFT))
            m_selectedStage = (m_selectedStage < 32) ? m_selectedStage + 1 : 1;
        if (input.isPressed(Action::MOVE_RIGHT))
            m_selectedStage = (m_selectedStage > 1)  ? m_selectedStage - 1 : 32;
        if (input.isPressed(Action::MOVE_UP))
            m_selectedStage = std::min(m_selectedStage + 4, 32);
        if (input.isPressed(Action::MOVE_DOWN))
            m_selectedStage = std::max(m_selectedStage - 4, 1);
        if (input.isPressed(Action::BACK))
            m_state = GameState::MENU;
        if (input.isPressed(Action::CONFIRM)) {
            StageManager::get().resetToStage(m_selectedStage);
            m_level  = 1;
            m_worldY = 0;
            m_waves.resetKillStats();
            { int bg = StageManager::get().currentDef().bgIndex;
              m_levelObjects.startStage(bg, islandsForCampaign(bg), m_terrainBigTex, m_terrainCarrierTex);
              m_background->setTextures({m_campaignBg[bg]}); }
            m_state = GameState::PLAYING;
            m_waves.startWave(StageManager::get().currentStage());
            m_player->setGodMode(true);
            AudioManager::get().playMusic("assets/sounds/bgm_stage.mp3");
        }
    }

    if (m_state == GameState::MENU && input.isPressed(Action::CONFIRM)) {
        StageManager::get().reset();
        m_level  = 1;
        m_worldY = 0;
        m_waves.resetKillStats();
        { int bg = StageManager::get().currentDef().bgIndex;
          m_levelObjects.startStage(bg, islandsForCampaign(bg), m_terrainBigTex, m_terrainCarrierTex);
          m_background->setTextures({m_campaignBg[bg]}); }
        m_state = GameState::PLAYING;
        m_waves.startWave(StageManager::get().currentStage());
        AudioManager::get().playMusic("assets/sounds/bgm_stage.mp3");
    }

    // Allow skipping the landing cutscene with Enter
    if (m_state == GameState::CARRIER_LANDING && input.isPressed(Action::CONFIRM)) {
        bool isFinal = (StageManager::get().currentStage() == 1);
        if (isFinal) { StageManager::get().updateHighScore(m_score); m_state = GameState::WIN; }
        else         { m_state = GameState::STAGE_TALLY; }
    }

    if (m_state == GameState::WIN && input.isPressed(Action::CONFIRM))
        resetToMenu();

    if (m_state == GameState::STAGE_TALLY && input.isPressed(Action::CONFIRM))
        advanceStage();

    if (m_state == GameState::GAMEOVER && input.isPressed(Action::CONFIRM))
        resetToMenu();
}

// -----------------------------------------------------------------------
void Game::update(float dt) {
    m_mouseIdleTimer += dt;
    if (m_mouseIdleTimer >= 2.0f) SDL_HideCursor();

    switch (m_state) {
    case GameState::MENU:
    case GameState::LEVEL_SELECT:
    case GameState::WIN:          m_menuTime += dt;     break;
    case GameState::PLAYING:          updatePlaying(dt);        break;
    case GameState::CARRIER_LANDING:  updateCarrierLanding(dt); break;
    case GameState::STAGE_TALLY:      updateStageTally(dt);     break;
    case GameState::GAMEOVER:
        m_landingTimer += dt;
        if (m_landingTimer >= 2.5f) resetToMenu();
        break;
    default: break;
    }
}

void Game::updatePlaying(float dt) {
    m_worldY += 80.f * dt;

    // Cloud scroll system — spawn and drift clouds down the screen
    {
        int bgIdx = std::clamp(StageManager::get().currentDef().bgIndex, 0, 7);
        // 0,1=Midway/Marshall, 4,5=Leyte/Saipan → light; 2,3=Attu/Rabaul, 6=IwoJima → dark; 7=Tokyo → sparse light
        int cloudType = (bgIdx == 2 || bgIdx == 3 || bgIdx == 6) ? 1 : 0;
        float spawnInterval = (bgIdx == 7) ? 6.0f : 3.0f;

        m_cloudSpawnTimer += dt;
        if (m_cloudSpawnTimer >= spawnInterval) {
            m_cloudSpawnTimer = 0.f;
            if (m_cloudTex[cloudType]) {
                CloudSprite cs;
                cs.texIdx = cloudType;
                cs.x      = (float)(std::rand() % (int)(LOGICAL_W + 80)) - 40.f;
                cs.y      = -80.f;
                cs.speed  = 25.f + (std::rand() % 20);
                cs.scale  = 1.5f + (std::rand() % 15) * 0.1f;
                cs.alpha  = (Uint8)(45 + std::rand() % 55);
                m_clouds.push_back(cs);
            }
        }

        for (auto& c : m_clouds) c.y += c.speed * dt;
        m_clouds.erase(std::remove_if(m_clouds.begin(), m_clouds.end(),
            [](const CloudSprite& c){ return c.y > LOGICAL_H + 120.f; }),
            m_clouds.end());
    }

    m_levelObjects.update(m_worldY, dt);
    m_player->update(dt);

    if (!m_player->isActive()) {
        StageManager::get().updateHighScore(m_score);
        m_landingTimer = 0.f;
        m_state = GameState::GAMEOVER;
        AudioManager::get().playSound("assets/sounds/game_over.mp3");
        return;
    }

    // Screen wipe power-up — clear all enemies
    if (m_player->hasScreenWipe()) {
        m_player->clearScreenWipe();
        for (auto& enemy : m_waves.enemies()) {
            if (enemy->isActive()) {
                spawnExplosion(enemy->bounds().x, enemy->bounds().y);
                enemy->setActive(false);
            }
        }
        m_bullets.clear();
    }

    // Player firing
    if (m_player->wantsToFire() && !m_player->isLooping()) {
        m_bullets.spawnPlayer(
            m_player->bounds().x + m_player->bounds().w * 0.5f - 5,
            m_player->bounds().y,
            m_player->fireLevel(), m_playerBulletTex);
        for (const auto& wm : m_player->wingmen()) {
            if (!wm.active) continue;
            m_bullets.spawnPlayer(
                m_player->bounds().x + m_player->bounds().w * 0.5f + wm.offsetX,
                m_player->bounds().y, 1, m_playerBulletTex);
        }
        AudioManager::get().playSound("assets/sounds/shoot_player.mp3");
    }

    {
        const auto& pb = m_player->bounds();
        float pcx = pb.x + pb.w * 0.5f;
        float pcy = pb.y + pb.h * 0.5f;
        m_waves.update(dt, pcx, pcy);
    }

    // Enemy firing
    if (!m_player->isBulletFrozen()) {
        static float enemyFireSoundTimer = 0;
        enemyFireSoundTimer -= dt;

        // Bullet speed ramps up in late campaigns
        int curStage = StageManager::get().currentStage();
        float bulletSpeed = ENEMY_BULLET_SPEED;
        if (curStage <= 6) bulletSpeed *= 1.25f;
        if (curStage <= 3) bulletSpeed *= 1.40f;

        for (auto& enemy : m_waves.enemies()) {
            if (!enemy->isActive()) continue;
            if (enemy->tryFire(dt)) {
                float ex = enemy->bounds().x + enemy->bounds().w * 0.5f;
                float ey = enemy->bounds().y + enemy->bounds().h;
                float vx = 0.f, vy = bulletSpeed;

                if (enemy->isSniper()) {
                    float pcx = m_player->bounds().x + m_player->bounds().w * 0.5f;
                    float pcy = m_player->bounds().y + m_player->bounds().h * 0.5f;
                    float dx = pcx - ex, dy = pcy - ey;
                    float len = std::sqrtf(dx*dx + dy*dy);
                    if (len < 1.f) { dx = 0.f; dy = 1.f; len = 1.f; }
                    vx = (dx / len) * bulletSpeed;
                    vy = (dy / len) * bulletSpeed;
                }

                m_bullets.spawnEnemy(ex, ey, vx, vy, m_enemyBulletTex);
                if (enemyFireSoundTimer <= 0) {
                    AudioManager::get().playSound("assets/sounds/shoot_enemy.mp3");
                    enemyFireSoundTimer = 0.12f;
                }
            }
        }
    }

    // Boss update + firing
    if (m_boss && m_boss->isActive()) {
        m_boss->update(dt);
        if (!m_player->isBulletFrozen()) {
            float pcx = m_player->bounds().x + m_player->bounds().w * 0.5f;
            float pcy = m_player->bounds().y + m_player->bounds().h * 0.5f;
            float bx, by, bvx, bvy;
            if (m_boss->tryFire(pcx, pcy, bx, by, bvx, bvy)) {
                // 5-bullet carpet fan: ±45° and ±22.5° around aimed direction
                float len = std::sqrtf(bvx*bvx + bvy*bvy);
                if (len < 1.f) len = 1.f;
                float nx = bvx / len, ny = bvy / len;
                static constexpr float kAngles[5] = {
                    -0.785f, -0.393f, 0.f, 0.393f, 0.785f  // -45°, -22.5°, 0°, +22.5°, +45°
                };
                for (float a : kAngles) {
                    float c = std::cosf(a), s = std::sinf(a);
                    m_bullets.spawnEnemy(bx, by,
                        (nx*c - ny*s) * ENEMY_BULLET_SPEED,
                        (nx*s + ny*c) * ENEMY_BULLET_SPEED,
                        m_enemyBulletTex);
                }
                AudioManager::get().playSound("assets/sounds/shoot_enemy.mp3");
            }
        }
        // Boss vs player bullets collision
        SDL_FRect bossBounds = m_boss->bounds();
        for (auto& b : m_bullets.bullets()) {
            if (!b.isActive() || b.owner() != BulletOwner::PLAYER) continue;
            if (m_boss->collidesWithRect(b.bounds())) {
                b.setActive(false);
                if (m_boss->hit()) {
                    for (int i = 0; i < 5; ++i)
                        spawnExplosion(bossBounds.x + (i * 20.f), bossBounds.y + (i * 15.f));
                    AudioManager::get().playSound("assets/sounds/explosion_large.mp3");
                    m_score += 50000;
                }
            }
        }
        // Boss vs player
        if (!m_player->isInvincible() && m_boss->collidesWithRect(m_player->bounds()))
            m_player->hit();
    }

    m_bullets.update(dt);

    // Player bullets vs destructible terrain (carrier ships)
    for (auto& t : m_levelObjects.objects()) {
        if (!t->isDestructible() || !t->isActive()) continue;
        for (auto& b : m_bullets.bullets()) {
            if (!b.isActive() || b.owner() != BulletOwner::PLAYER) continue;
            if (t->collidesWithRect(b.bounds())) {
                b.setActive(false);
                if (t->hit()) {
                    spawnExplosion(t->bounds().x + t->bounds().w * 0.3f,
                                   t->bounds().y + t->bounds().h * 0.3f);
                    spawnExplosion(t->bounds().x + t->bounds().w * 0.7f,
                                   t->bounds().y + t->bounds().h * 0.6f);
                    AudioManager::get().playSound("assets/sounds/explosion_large.mp3");
                    m_score += 5000;
                }
            }
        }
    }

    for (auto& ex : m_explosions) ex.anim.update(dt);
    m_explosions.erase(
        std::remove_if(m_explosions.begin(), m_explosions.end(),
            [](const Explosion& ex){ return ex.anim.isFinished(); }),
        m_explosions.end());

    m_collision.check(*m_player, m_bullets, m_waves);

    bool waveDone = m_waves.isWaveCleared();
    bool bossDone = !m_boss || !m_boss->isActive();

    if (waveDone) {
        if (StageManager::get().currentDef().hasBoss && !m_bossSpawned) {
            spawnBoss();
        } else if (bossDone) {
            m_stageKillCount    = m_waves.killCount();
            m_stageTotalEnemies = m_waves.totalSpawned();
            m_tallyTimer        = 0;
            m_tallyBonusAdded   = false;
            m_bullets.clear();
            m_boss.reset();
            m_bossSpawned = false;
            // Boss stages get the carrier landing cutscene; non-boss stages skip straight to tally
            if (StageManager::get().currentDef().hasBoss) {
                m_landingTimer  = 0.f;
                m_landingPlaneY = (float)LOGICAL_H + 80.f;  // start below screen, fly upward
                m_state = GameState::CARRIER_LANDING;
            } else {
                m_state = GameState::STAGE_TALLY;
            }
        }
    }
}

void Game::updateCarrierLanding(float dt) {
    m_landingTimer += dt;

    // Plane climbs from off-screen bottom to the carrier deck over 2 seconds
    static constexpr float DESCENT_DUR = 2.0f;
    static constexpr float HOLD_DUR    = 1.2f;  // pause on deck before tally
    static constexpr float TOTAL_DUR   = DESCENT_DUR + HOLD_DUR;

    // Touchdown point: upper portion of the carrier deck
    float deckY  = LOGICAL_H * 0.42f;
    float deckH  = (float)LOGICAL_W * 0.55f;
    float landY  = deckY + deckH * 0.30f;   // ~1/3 down the deck
    float startY = (float)LOGICAL_H + 80.f;
    if (m_landingTimer < DESCENT_DUR) {
        float t = m_landingTimer / DESCENT_DUR;
        t = 1.f - (1.f - t) * (1.f - t);   // ease-out: slows as it nears the deck
        m_landingPlaneY = startY + t * (landY - startY);  // y decreases — flies upward
    } else {
        m_landingPlaneY = landY;
    }

    if (m_landingTimer >= TOTAL_DUR) {
        bool isFinal = (StageManager::get().currentStage() == 1);
        if (isFinal) { StageManager::get().updateHighScore(m_score); m_state = GameState::WIN; }
        else         { m_state = GameState::STAGE_TALLY; }
    }
}

void Game::updateStageTally(float dt) {
    m_tallyTimer += dt;
    if (m_tallyTimer >= 4.0f)
        advanceStage();
}

// -----------------------------------------------------------------------
void Game::render() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    switch (m_state) {
    case GameState::MENU:        renderMenu();        break;
    case GameState::LEVEL_SELECT:renderLevelSelect(); break;
    case GameState::PLAYING:         renderPlaying();        break;
    case GameState::PAUSED:          renderPaused();         break;
    case GameState::CARRIER_LANDING: renderCarrierLanding(); break;
    case GameState::STAGE_TALLY:     renderStageTally();     break;
    case GameState::WIN:             renderWin();            break;
    case GameState::GAMEOVER:    renderGameOver();    break;
    }

    SDL_RenderPresent(m_renderer);
}

void Game::renderPlaying() {
    // Per-campaign fill color shown beneath/between background tiles
    static constexpr SDL_Color kCampaignColor[8] = {
        { 25,  80, 140, 255},  // 0 Midway    — deep Pacific blue
        { 15, 130, 150, 255},  // 1 Marshall  — turquoise atoll
        { 35,  50,  70, 255},  // 2 Attu      — cold slate-gray
        { 20,  55,  45, 255},  // 3 Rabaul    — dark volcanic green
        { 15,  40,  45, 255},  // 4 Leyte     — oily battle water
        { 50,  55,  60, 255},  // 5 Saipan    — industrial harbor
        { 35,  38,  42, 255},  // 6 Iwo Jima  — volcanic ash gray
        { 55,  58,  52, 255},  // 7 Tokyo     — urban gray-green
    };
    int bgIdx = std::clamp(StageManager::get().currentDef().bgIndex, 0, 7);
    const SDL_Color& c = kCampaignColor[bgIdx];
    SDL_SetRenderDrawColor(m_renderer, c.r, c.g, c.b, 255);
    SDL_RenderFillRect(m_renderer, nullptr);

    // Islands / terrain — sea level, drawn first
    m_levelObjects.render(m_renderer);

    // Clouds — atmospheric layer above the sea, below the aircraft
    for (const auto& cl : m_clouds) {
        SDL_Texture* tex = m_cloudTex[cl.texIdx];
        if (!tex) continue;
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);
        float w = tw * cl.scale;
        float h = th * cl.scale;
        SDL_SetTextureAlphaMod(tex, cl.alpha);
        SDL_FRect dst = {cl.x - w * 0.5f, cl.y, w, h};
        SDL_RenderTexture(m_renderer, tex, nullptr, &dst);
        SDL_SetTextureAlphaMod(tex, 255);
    }

    m_waves.render(m_renderer);
    if (m_boss) m_boss->render(m_renderer);
    m_bullets.render(m_renderer);
    m_player->render(m_renderer);
    for (auto& ex : m_explosions)
        ex.anim.render(m_renderer, ex.x, ex.y, 48.f, 48.f);

    m_hud->render(m_renderer, m_score, m_player->lives(),
                  StageManager::get().currentStage(),
                  m_player->shieldLevel(),
                  m_player->loopsRemaining(),
                  StageManager::get().highScore());

    if (m_player->godMode())
        renderText("TRAINING", 4, LOGICAL_H * 0.5f - 8, {255, 80, 80, 200}, 14);
}

void Game::renderMenu() {
    if (m_menuBg) {
        SDL_FRect dst = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
        SDL_RenderTexture(m_renderer, m_menuBg, nullptr, &dst);
    }

    // Dark panel at bottom for button readability
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 170);
    SDL_FRect panel = {0, LOGICAL_H - 115.f, (float)LOGICAL_W, 115.f};
    SDL_RenderFillRect(m_renderer, &panel);

    // "PLAY" button — pulsing gold outline box + label
    float pulse = 0.5f + 0.5f * std::sinf(m_menuTime * 3.2f);
    Uint8 pa = (Uint8)(160 + 95 * pulse);
    SDL_Color playCol = {255, 220, 0, pa};
    // key box
    SDL_SetRenderDrawColor(m_renderer, 255, 220, 0, pa);
    SDL_FRect playBox = {28.f, LOGICAL_H - 100.f, 64.f, 24.f};
    SDL_RenderRect(m_renderer, &playBox);
    renderText("ENTER", 34.f, LOGICAL_H - 97.f, playCol, 11);
    renderText("PLAY", 104.f, LOGICAL_H - 97.f, playCol, 16);

    // "TRAINING" button — static grey
    SDL_SetRenderDrawColor(m_renderer, 180, 180, 180, 180);
    SDL_FRect trainBox = {28.f, LOGICAL_H - 66.f, 28.f, 22.f};
    SDL_RenderRect(m_renderer, &trainBox);
    renderText("T", 36.f, LOGICAL_H - 63.f, {180, 180, 180, 200}, 12);
    renderText("TRAINING / LEVEL SELECT", 68.f, LOGICAL_H - 63.f, {180, 180, 180, 200}, 11);

    // Hi-score
    if (StageManager::get().highScore() > 0) {
        std::string hs = "HI-SCORE  " + std::to_string(StageManager::get().highScore());
        renderText(hs, LOGICAL_W * 0.5f - 65.f, LOGICAL_H - 26.f, {255, 220, 0, 200}, 11);
    }
}

void Game::renderLevelSelect() {
    // Background
    if (m_menuBg) {
        SDL_FRect dst = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
        SDL_RenderTexture(m_renderer, m_menuBg, nullptr, &dst);
    }
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 190);
    SDL_FRect overlay = {0, LOGICAL_H * 0.38f, (float)LOGICAL_W, LOGICAL_H * 0.62f};
    SDL_RenderFillRect(m_renderer, &overlay);

    float cx = LOGICAL_W * 0.5f;

    // Header
    renderText("TRAINING MODE", cx - 78.f, LOGICAL_H * 0.40f, {255, 80, 80, 255}, 14);
    renderText("STAGE SELECT", cx - 65.f, LOGICAL_H * 0.40f + 20.f, {200, 200, 200, 200}, 12);

    // Stage number — large and gold
    const StageDef& def = StageManager::get().getDef(m_selectedStage);
    std::string stageStr = std::to_string(m_selectedStage);
    renderText(stageStr, cx - stageStr.size() * 11.f, LOGICAL_H * 0.52f, {255, 215, 0, 255}, 36);

    // Campaign + boss flag
    std::string campaign = def.campaign;
    renderText(campaign, cx - campaign.size() * 4.5f, LOGICAL_H * 0.62f, {200, 220, 255, 255}, 14);
    if (def.hasBoss)
        renderText("BOSS STAGE", cx - 45.f, LOGICAL_H * 0.62f + 20.f, {255, 80, 80, 220}, 12);

    // Pulsing navigation arrows
    float pulse = 0.5f + 0.5f * std::sinf(m_menuTime * 4.f);
    Uint8 arrowAlpha = (Uint8)(140 + 115 * pulse);
    SDL_Color arrowCol = {255, 215, 0, arrowAlpha};
    renderText("<<", cx - 95.f, LOGICAL_H * 0.52f + 6.f, arrowCol, 18);
    renderText(">>", cx + 62.f, LOGICAL_H * 0.52f + 6.f, arrowCol, 18);

    // Navigation hints
    renderText("LEFT / RIGHT  by stage",   cx - 88.f, LOGICAL_H * 0.74f, {160, 160, 160, 200}, 11);
    renderText("UP / DOWN  by campaign",   cx - 85.f, LOGICAL_H * 0.74f + 18.f, {160, 160, 160, 200}, 11);

    // Action buttons
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 200);
    SDL_FRect startBox = {cx - 85.f, LOGICAL_H * 0.83f, 80.f, 22.f};
    SDL_RenderRect(m_renderer, &startBox);
    renderText("ENTER", cx - 80.f, LOGICAL_H * 0.83f + 3.f, {255, 215, 0, 220}, 11);
    renderText("START HERE", cx + 2.f, LOGICAL_H * 0.83f + 3.f, {255, 255, 255, 220}, 12);

    SDL_SetRenderDrawColor(m_renderer, 160, 160, 160, 180);
    SDL_FRect backBox = {cx - 85.f, LOGICAL_H * 0.83f + 30.f, 80.f, 22.f};
    SDL_RenderRect(m_renderer, &backBox);
    renderText("BKSP", cx - 79.f, LOGICAL_H * 0.83f + 33.f, {160, 160, 160, 200}, 11);
    renderText("BACK TO MENU", cx + 2.f, LOGICAL_H * 0.83f + 33.f, {160, 160, 160, 200}, 11);
}

void Game::renderPaused() {
    renderPlaying();
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 120);
    SDL_FRect overlay = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
    SDL_RenderFillRect(m_renderer, &overlay);
    renderText("PAUSED", LOGICAL_W * 0.5f - 40, LOGICAL_H * 0.5f - 14, {255, 255, 255, 255}, 20);
}

void Game::renderCarrierLanding() {
    // Ocean fill beneath carrier
    SDL_SetRenderDrawColor(m_renderer, 15, 55, 100, 255);
    SDL_RenderFillRect(m_renderer, nullptr);

    // Carrier deck — fill most of the screen width, centered vertically in lower half
    float deckW = (float)LOGICAL_W;
    float deckH = deckW * 0.55f;
    float deckX = 0.f;
    float deckY = LOGICAL_H * 0.42f;

    if (m_carrierDeckTex) {
        SDL_FRect deckDst = {deckX, deckY, deckW, deckH};
        SDL_RenderTexture(m_renderer, m_carrierDeckTex, nullptr, &deckDst);
    } else {
        // Fallback: draw a simple carrier shape with SDL primitives
        SDL_SetRenderDrawColor(m_renderer, 70, 75, 80, 255);
        SDL_FRect deck = {0, deckY, deckW, deckH};
        SDL_RenderFillRect(m_renderer, &deck);
        // Centerline stripe
        SDL_SetRenderDrawColor(m_renderer, 220, 220, 200, 255);
        SDL_FRect stripe = {deckX + deckW * 0.47f, deckY, 8, deckH};
        SDL_RenderFillRect(m_renderer, &stripe);
        // Arresting wires (3 horizontal lines across deck)
        SDL_SetRenderDrawColor(m_renderer, 180, 160, 60, 255);
        for (int i = 1; i <= 3; ++i) {
            float wy = deckY + deckH * (0.25f + i * 0.12f);
            SDL_FRect wire = {deckX + 10, wy, deckW - 20, 3};
            SDL_RenderFillRect(m_renderer, &wire);
        }
        // Island superstructure
        SDL_SetRenderDrawColor(m_renderer, 90, 85, 80, 255);
        SDL_FRect island = {deckX + deckW * 0.72f, deckY + 8, 40, 60};
        SDL_RenderFillRect(m_renderer, &island);
    }

    // Player plane descending onto the carrier
    SDL_Texture* planeTex = AssetManager::get().texture("assets/PNG/playerShip_p38.png");
    if (planeTex) {
        float pw, ph;
        SDL_GetTextureSize(planeTex, &pw, &ph);
        float scale = 1.5f;
        float pw2 = pw * scale, ph2 = ph * scale;
        float px = LOGICAL_W * 0.5f - pw2 * 0.5f;
        SDL_FRect planeDst = {px, m_landingPlaneY, pw2, ph2};
        SDL_RenderTexture(m_renderer, planeTex, nullptr, &planeDst);
    }

    // "MISSION COMPLETE" banner — pops in after plane has climbed onto screen
    if (m_landingTimer > 0.6f) {
        float age   = m_landingTimer - 0.6f;
        Uint8 alpha = (Uint8)std::min(255.f, age * 400.f);

        // POW flash: large white pulse that fades quickly
        if (age < 0.35f) {
            Uint8 flashA = (Uint8)((1.f - age / 0.35f) * 200.f);
            SDL_SetRenderDrawColor(m_renderer, 255, 240, 100, flashA);
            SDL_FRect flash = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
            SDL_RenderFillRect(m_renderer, &flash);
        }

        std::string campaign = StageManager::get().currentDef().campaign;
        std::string secured  = campaign + " SECURED";
        // Large "MISSION COMPLETE" centred near top
        renderText("MISSION COMPLETE",
                   LOGICAL_W * 0.5f - 118.f, 28.f,
                   {255, 255, 255, alpha}, 26);
        // Campaign sub-label in gold below
        renderText(secured,
                   LOGICAL_W * 0.5f - (float)(secured.size() * 6.5f), 64.f,
                   {255, 215, 0, alpha}, 20);
    }
}

void Game::renderStageTally() {
    // Dark background
    if (m_menuBg) {
        SDL_FRect dst = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
        SDL_RenderTexture(m_renderer, m_menuBg, nullptr, &dst);
    }
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 180);
    SDL_FRect box = {10, 160, 340, 300};
    SDL_RenderFillRect(m_renderer, &box);

    int stage = StageManager::get().currentStage();
    std::string hdr = "STAGE " + std::to_string(stage) + " CLEAR";
    renderText(hdr, LOGICAL_W * 0.5f - (float)(hdr.size() * 5), 175, {255, 255, 100, 255}, 22);

    // Shooting %
    int pct = (m_stageTotalEnemies > 0)
              ? (m_stageKillCount * 100 / m_stageTotalEnemies) : 0;
    std::string pctTxt = "SHOOTING DOWN  " + std::to_string(pct) + "%";
    renderText(pctTxt, 30, 230, {255, 255, 255, 255}, 16);

    // Loop bonus
    int loopBonus = m_player->loopsRemaining() * SCORE_LOOP_BONUS;
    std::string loopTxt = "LOOP BONUS  " + std::to_string(m_player->loopsRemaining())
                        + " x 1000 = " + std::to_string(loopBonus);
    renderText(loopTxt, 30, 270, {255, 255, 255, 255}, 16);

    // Perfect bonus
    if (pct == 100)
        renderText("SPECIAL BONUS  50000", 30, 310, {255, 200, 0, 255}, 16);

    // Add bonuses exactly once
    if (!m_tallyBonusAdded) {
        m_tallyBonusAdded = true;
        m_score += loopBonus;
        if (pct == 100) m_score += SCORE_PERFECT_BONUS;
        AudioManager::get().playSound("assets/sounds/stage_clear.mp3");
    }

    renderText("PRESS ENTER TO CONTINUE", LOGICAL_W * 0.5f - 110, 390,
               {200, 200, 200, 255}, 13);
}

void Game::renderGameOver() {
    if (m_menuBg) {
        SDL_FRect dst = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
        SDL_RenderTexture(m_renderer, m_menuBg, nullptr, &dst);
    }
    renderText("GAME OVER", LOGICAL_W * 0.5f - 65, 260, {255, 60, 60, 255}, 28);
    std::string sc = "SCORE  " + std::to_string(m_score);
    renderText(sc, LOGICAL_W * 0.5f - 60, 310, {255, 255, 255, 255}, 20);
    renderText("PRESS ENTER", LOGICAL_W * 0.5f - 55, 360, {200, 200, 200, 255}, 16);
}

void Game::renderWin() {
    // Intro screen as backdrop
    if (m_menuBg) {
        SDL_FRect dst = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
        SDL_RenderTexture(m_renderer, m_menuBg, nullptr, &dst);
    }

    // Vignette overlay — dark edges, bright center
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 160);
    SDL_RenderFillRect(m_renderer, nullptr);

    float cx = LOGICAL_W * 0.5f;

    // Gold ribbon behind the title
    SDL_SetRenderDrawColor(m_renderer, 160, 120, 0, 200);
    SDL_FRect ribbon = {0, 84, (float)LOGICAL_W, 54};
    SDL_RenderFillRect(m_renderer, &ribbon);
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 220);
    SDL_FRect ribbonTop = {0, 84, (float)LOGICAL_W, 3};
    SDL_FRect ribbonBot = {0, 135, (float)LOGICAL_W, 3};
    SDL_RenderFillRect(m_renderer, &ribbonTop);
    SDL_RenderFillRect(m_renderer, &ribbonBot);

    // Main title
    renderText("VICTORY!", cx - 56, 92, {255, 240, 80, 255}, 30);

    // Subtitle
    renderText("PROMOTED TO GENERAL", cx - 95, 148, {255, 200, 60, 255}, 16);

    // Horizontal rule
    SDL_SetRenderDrawColor(m_renderer, 200, 160, 40, 180);
    SDL_FRect sep = {30, 178, LOGICAL_W - 60, 2};
    SDL_RenderFillRect(m_renderer, &sep);

    // Campaign cleared line
    renderText("ALL 8 CAMPAIGNS CLEARED", cx - 110, 190, {200, 230, 255, 255}, 16);

    // Final score
    std::string scoreStr = "FINAL SCORE   " + std::to_string(m_score);
    renderText(scoreStr, cx - (float)(scoreStr.size() * 5.f), 220, {255, 255, 255, 255}, 18);

    // Hi-score line
    int hi = StageManager::get().highScore();
    if (hi > 0) {
        std::string hsStr = "HI-SCORE   " + std::to_string(hi);
        renderText(hsStr, cx - (float)(hsStr.size() * 4.5f), 248, {255, 215, 0, 220}, 16);
    }

    // Rank badge
    const char* rankLabel;
    SDL_Color   rankCol;
    if      (m_score >= 500000) { rankLabel = "S  ACE OF ACES";      rankCol = {255, 100, 100, 255}; }
    else if (m_score >= 300000) { rankLabel = "A  WING COMMANDER";   rankCol = {255, 180,  60, 255}; }
    else if (m_score >= 150000) { rankLabel = "B  SQUADRON LEADER";  rankCol = {120, 220, 120, 255}; }
    else                        { rankLabel = "C  FLIGHT OFFICER";   rankCol = {160, 200, 255, 255}; }

    SDL_SetRenderDrawColor(m_renderer, rankCol.r/4, rankCol.g/4, rankCol.b/4, 200);
    SDL_FRect rankBox = {cx - 100, 278, 200, 30};
    SDL_RenderFillRect(m_renderer, &rankBox);
    SDL_SetRenderDrawColor(m_renderer, rankCol.r, rankCol.g, rankCol.b, 200);
    SDL_RenderRect(m_renderer, &rankBox);
    renderText("RANK", cx - 96, 283, rankCol, 11);
    renderText(rankLabel, cx - 60, 283, rankCol, 14);

    // Carrier deck thumbnail if available — small decorative image
    if (m_carrierDeckTex) {
        SDL_FRect mini = {cx - 55, 322, 110, 44};
        SDL_RenderTexture(m_renderer, m_carrierDeckTex, nullptr, &mini);
        SDL_SetRenderDrawColor(m_renderer, 200, 180, 100, 120);
        SDL_RenderRect(m_renderer, &mini);
    }

    // Pulsing "press enter" prompt
    float pulse = 0.5f + 0.5f * std::sinf(m_menuTime * 3.2f);
    Uint8 pa = (Uint8)(130 + 125 * pulse);
    renderText("PRESS ENTER TO RETURN", cx - 100, 378, {255, 215, 0, pa}, 14);
}

// -----------------------------------------------------------------------
void Game::spawnExplosion(float x, float y) {
    m_explosions.push_back({
        SpriteAnimation(m_explosionFrames, 24.0f, false),
        x, y
    });
    AudioManager::get().playSound("assets/sounds/explosion_small.mp3");
}

void Game::spawnBoss() {
    m_bossSpawned = true;
    int stageNum  = StageManager::get().currentDef().stageNumber;
    int bossIndex = std::clamp((29 - stageNum) / 4 + 1, 1, 8);
    // Boss sprite progression: Ayako (1-2) → Cruiser (3-4) → Kaga carrier (5-6) → Yamato (7-8)
    // Plane sprites are stored facing north and need a vertical flip; ship sprites do not.
    SDL_Texture* bossTex;
    bool flipV = true;
    if      (bossIndex >= 7 && m_bossYamatoTex)  { bossTex = m_bossYamatoTex;  flipV = false; }
    else if (bossIndex >= 5 && m_bossKagaTex)     { bossTex = m_bossKagaTex;    flipV = false; }
    else if (bossIndex >= 3 && m_bossCruiserTex)  { bossTex = m_bossCruiserTex; flipV = false; }
    else                                             bossTex = m_bossTex;
    m_boss = std::make_unique<Boss>(bossTex, bossIndex, flipV);
    AudioManager::get().playSound("assets/sounds/boss_warning.mp3");
    AudioManager::get().playMusic("assets/sounds/bgm_boss.mp3");
}

std::vector<SDL_Texture*> Game::islandsForCampaign(int bgIdx) const {
    std::vector<SDL_Texture*> result;
    auto add = [&](const std::vector<SDL_Texture*>& src) {
        result.insert(result.end(), src.begin(), src.end());
    };
    switch (bgIdx) {
    case 0: add(m_islandsPalm);   add(m_islandsAtoll);   break;  // Midway: tropical + atolls
    case 1: add(m_islandsAtoll);  add(m_islandsPalm);    break;  // Marshall: atolls + palms
    case 2: add(m_islandsForest); add(m_islandsAtoll);   break;  // Attu: rocky forest + atolls
    case 3: add(m_islandsVolcano);add(m_islandsForest);  break;  // Rabaul: volcanic + jungle
    case 4: add(m_islandsPalm);   add(m_islandsForest);  break;  // Leyte: palm + jungle
    case 5: add(m_islandsPalm);   add(m_islandsAtoll);   break;  // Saipan: tropical military
    case 6: add(m_islandsVolcano);                        break;  // Iwo Jima: volcanic only
    case 7: /* Tokyo: no small islands (all carriers in area7 script) */  break;
    default:add(m_terrainSmallTextures);                  break;
    }
    // Fallback: if nothing loaded yet, return master list
    if (result.empty() && !m_terrainSmallTextures.empty())
        result = m_terrainSmallTextures;
    return result;
}

void Game::advanceStage() {
    StageManager::get().advance();
    ++m_level;

    m_worldY = 0;
    int area = StageManager::get().currentDef().bgIndex;
    m_levelObjects.startStage(area,
        islandsForCampaign(area), m_terrainBigTex, m_terrainCarrierTex);
    m_background->setTextures({m_campaignBg[area]});

    m_player->resetForNewStage();
    m_waves.resetKillStats();
    m_waves.startWave(StageManager::get().currentStage());

    const char* music;
    if (StageManager::get().currentDef().hasBoss)
        music = "assets/sounds/bgm_boss.mp3";
    else if (area >= 2)
        music = "assets/sounds/bgm_stage2.mp3";
    else
        music = "assets/sounds/bgm_stage.mp3";
    AudioManager::get().playMusic(music);

    m_state = GameState::PLAYING;
}

void Game::resetToMenu() {
    StageManager::get().updateHighScore(m_score);
    m_score       = 0;
    m_level       = 1;
    m_landingTimer = 0.f;
    m_state       = GameState::MENU;
    m_waves.clear();
    m_bullets.clear();
    m_levelObjects.clear();
    m_boss.reset();
    m_bossSpawned = false;
    m_worldY      = 0;
    m_explosions.clear();
    m_clouds.clear();
    resetPlayer();
    AudioManager::get().stopMusic();
}

void Game::resetPlayer() {
    m_score = 0;
    auto& am = AssetManager::get();
    SDL_Texture* shipTex    = am.texture("assets/PNG/playerShip_p38.png");
    SDL_Texture* wingmanTex = am.texture("assets/PNG/playerWingman.png");
    std::array<SDL_Texture*, 3> damageTex = {nullptr, nullptr, nullptr};
    m_player = std::make_unique<Player>(
        LOGICAL_W * 0.5f - 24, LOGICAL_H - 120.f,
        shipTex, damageTex, wingmanTex);
}

void Game::renderText(const std::string& text, float x, float y,
                      SDL_Color col, int ptSize) {
    TTF_Font* f = AssetManager::get().font(
        "assets/Bonus/kenvector_future.ttf", ptSize);
    if (!f) return;
    SDL_Surface* s = TTF_RenderText_Blended(f, text.c_str(), text.size(), col);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(m_renderer, s);
    SDL_DestroySurface(s);
    if (!t) return;
    float tw, th;
    SDL_GetTextureSize(t, &tw, &th);
    SDL_FRect d = {x, y, tw, th};
    SDL_RenderTexture(m_renderer, t, nullptr, &d);
    SDL_DestroyTexture(t);
}

// -----------------------------------------------------------------------
void Game::shutdown() {
    m_player.reset();
    m_boss.reset();
    m_background.reset();
    m_hud.reset();
    m_waves.clear();
    m_bullets.clear();
    m_levelObjects.clear();
    m_explosions.clear();
    m_explosionFrames.clear();

    AudioManager::get().shutdown();
    AssetManager::get().shutdown();

    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }

    TTF_Quit();
    SDL_Quit();
}
