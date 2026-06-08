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

    // Single ocean panel repeated endlessly — terrain objects overlay on top
    SDL_Texture* oceanTex = am.texture("assets/PNG/Backgrounds/bg_ocean.png");
    m_menuBg = oceanTex;
    m_background = std::make_unique<Background>(
        std::vector<SDL_Texture*>{oceanTex, oceanTex, oceanTex}, 80.f);

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

    // Enemy textures — Zero variants + Betty bombers
    SDL_Texture* zeroTex  = am.texture("assets/PNG/Enemies/enemyZero.png");
    SDL_Texture* zeroRed  = am.texture("assets/PNG/Enemies/enemyZero_red.png");
    SDL_Texture* zeroBlue = am.texture("assets/PNG/Enemies/enemyZero_blue.png");
    SDL_Texture* betty    = am.texture("assets/PNG/Enemies/enemyBetty.png");
    SDL_Texture* nell     = am.texture("assets/PNG/Enemies/enemyNell.png");
    SDL_Texture* black[5], *red[5], *blue[5], *green[5];
    for (int i = 0; i < 5; ++i) {
        black[i] = zeroTex  ? zeroTex  : nullptr;
        red[i]   = zeroRed  ? zeroRed  : zeroTex;
        blue[i]  = zeroBlue ? zeroBlue : zeroTex;
        green[i] = betty    ? betty    : nell;
    }
    m_waves.loadTextures(black, red, blue, green, nell);

    // Bullet sprites
    m_playerBulletTex = am.texture("assets/PNG/Lasers/playerBullet.png");
    m_enemyBulletTex  = am.texture("assets/PNG/Lasers/enemyBullet.png");

    // Boss
    m_bossTex = am.texture("assets/PNG/Enemies/bossAyako.png");

    // Terrain objects (islands + carrier ships)
    m_terrainSmallTex   = am.texture("assets/PNG/Terrain/small_island.png");
    m_terrainBigTex     = am.texture("assets/PNG/Terrain/big_island.png");
    m_terrainCarrierTex = am.texture("assets/PNG/Terrain/carrier_ship.png");

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

    AudioManager::get().playMusic("assets/sounds/bgm_menu.mp3");

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
        input.update(e);
    }

    if (input.isPressed(Action::TRAINING) && m_player) {
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

    if (m_state == GameState::MENU && input.isPressed(Action::CONFIRM)) {
        StageManager::get().reset();
        m_level  = 1;
        m_worldY = 0;
        m_waves.resetKillStats();
        m_levelObjects.startStage(StageManager::get().currentDef().bgIndex,
            m_terrainSmallTex, m_terrainBigTex, m_terrainCarrierTex);
        m_state = GameState::PLAYING;
        m_waves.startWave(m_level);
        AudioManager::get().playMusic("assets/sounds/bgm_stage.mp3");
    }

    if (m_state == GameState::STAGE_TALLY && input.isPressed(Action::CONFIRM)) {
        advanceStage();
    }

    if (m_state == GameState::GAMEOVER && input.isPressed(Action::CONFIRM)) {
        StageManager::get().updateHighScore(m_score);
        m_score  = 0;
        m_level  = 1;
        m_state  = GameState::MENU;
        m_waves.clear();
        m_bullets.clear();
        m_levelObjects.clear();
        m_boss.reset();
        m_bossSpawned = false;
        m_worldY      = 0;
        m_explosions.clear();
        resetPlayer();
    }
}

// -----------------------------------------------------------------------
void Game::update(float dt) {
    switch (m_state) {
    case GameState::PLAYING:     updatePlaying(dt);     break;
    case GameState::STAGE_TALLY: updateStageTally(dt);  break;
    default: break;
    }
}

void Game::updatePlaying(float dt) {
    m_background->update(dt);
    m_worldY += 80.f * dt;
    m_levelObjects.update(m_worldY, dt);
    m_player->update(dt);

    if (!m_player->isActive()) {
        StageManager::get().updateHighScore(m_score);
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

    m_waves.update(dt);

    // Enemy firing
    if (!m_player->isBulletFrozen()) {
        static float enemyFireSoundTimer = 0;
        enemyFireSoundTimer -= dt;
        for (auto& enemy : m_waves.enemies()) {
            if (!enemy->isActive()) continue;
            if (enemy->tryFire(dt)) {
                m_bullets.spawnEnemy(
                    enemy->bounds().x + enemy->bounds().w * 0.5f,
                    enemy->bounds().y + enemy->bounds().h,
                    0, ENEMY_BULLET_SPEED, m_enemyBulletTex);
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
                m_bullets.spawnEnemy(bx, by, bvx, bvy, m_enemyBulletTex);
                // Boss fires three-shot burst
                m_bullets.spawnEnemy(bx, by, -bvx, bvy, m_enemyBulletTex);
                m_bullets.spawnEnemy(bx, by, 0, bvy * 0.9f, m_enemyBulletTex);
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
            m_state = GameState::STAGE_TALLY;
        }
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
    case GameState::PLAYING:     renderPlaying();     break;
    case GameState::PAUSED:      renderPaused();      break;
    case GameState::STAGE_TALLY: renderStageTally();  break;
    case GameState::GAMEOVER:    renderGameOver();    break;
    }

    SDL_RenderPresent(m_renderer);
}

void Game::renderPlaying() {
    m_background->render(m_renderer);
    m_levelObjects.render(m_renderer);
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
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 160);
    SDL_FRect box = {20, 220, 320, 160};
    SDL_RenderFillRect(m_renderer, &box);

    renderText("1942", LOGICAL_W * 0.5f - 50, 235, {255, 255, 255, 255}, 36);
    renderText("PRESS ENTER TO PLAY", LOGICAL_W * 0.5f - 110, 300, {255, 220, 0, 255}, 16);

    if (StageManager::get().highScore() > 0) {
        std::string hs = "HI " + std::to_string(StageManager::get().highScore());
        renderText(hs, LOGICAL_W * 0.5f - 40, 340, {200, 200, 200, 255}, 14);
    }
}

void Game::renderPaused() {
    renderPlaying();
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 120);
    SDL_FRect overlay = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
    SDL_RenderFillRect(m_renderer, &overlay);
    renderText("PAUSED", LOGICAL_W * 0.5f - 40, LOGICAL_H * 0.5f - 14, {255, 255, 255, 255}, 20);
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
    m_boss = std::make_unique<Boss>(m_bossTex, bossIndex);
    AudioManager::get().playSound("assets/sounds/boss_warning.mp3");
    AudioManager::get().playMusic("assets/sounds/bgm_boss.mp3");
}

void Game::advanceStage() {
    StageManager::get().advance();
    ++m_level;

    m_worldY = 0;
    int area = StageManager::get().currentDef().bgIndex;
    m_levelObjects.startStage(area,
        m_terrainSmallTex, m_terrainBigTex, m_terrainCarrierTex);

    m_player->resetForNewStage();
    m_waves.resetKillStats();
    m_waves.startWave(m_level);

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
