#include "Game.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "InputManager.h"
#include "StageManager.h"
#include "Constants.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
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
                                 480, 640,
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

    // Background textures for each campaign tier
    m_bgTextures[0] = am.texture("assets/Backgrounds/darkPurple.png");
    m_bgTextures[1] = am.texture("assets/Backgrounds/blue.png");
    m_bgTextures[2] = am.texture("assets/Backgrounds/purple.png");
    m_bgTextures[3] = am.texture("assets/Backgrounds/black.png");

    m_background = std::make_unique<Background>(m_bgTextures[0], 70.f);

    // Explosion frames
    m_explosionFrames.reserve(20);
    for (int i = 0; i < 20; ++i) {
        char buf[64];
        SDL_snprintf(buf, sizeof(buf), "assets/PNG/Effects/fire%02d.png", i);
        m_explosionFrames.push_back(am.texture(buf));
    }

    // Player — use PixelLab P-38 if available, fall back to original
    SDL_Texture* shipTex = am.texture("assets/PNG/playerShip_p38.png");
    if (!shipTex) shipTex = am.texture("assets/PNG/playerShip1_orange.png");

    std::array<SDL_Texture*, 3> damageTex = {
        am.texture("assets/PNG/Damage/playerShip1_damage1.png"),
        am.texture("assets/PNG/Damage/playerShip1_damage2.png"),
        am.texture("assets/PNG/Damage/playerShip1_damage3.png"),
    };
    m_player = std::make_unique<Player>(
        LOGICAL_W * 0.5f - 33, LOGICAL_H - 120.f,
        shipTex, damageTex);

    // Enemy textures — use PixelLab Zero for SMALL if available
    SDL_Texture* zeroTex = am.texture("assets/PNG/Enemies/enemyZero.png");
    SDL_Texture* black[5], *red[5], *blue[5], *green[5];
    for (int i = 0; i < 5; ++i) {
        char buf[64];
        SDL_snprintf(buf, sizeof(buf), "assets/PNG/Enemies/enemyBlack%d.png", i + 1);
        black[i] = zeroTex ? zeroTex : am.texture(buf);
        SDL_snprintf(buf, sizeof(buf), "assets/PNG/Enemies/enemyRed%d.png", i + 1);
        red[i] = am.texture(buf);
        SDL_snprintf(buf, sizeof(buf), "assets/PNG/Enemies/enemyBlue%d.png", i + 1);
        blue[i] = am.texture(buf);
        SDL_snprintf(buf, sizeof(buf), "assets/PNG/Enemies/enemyGreen%d.png", i + 1);
        green[i] = am.texture(buf);
    }
    SDL_Texture* ufoTex = am.texture("assets/PNG/ufoRed.png");
    m_waves.loadTextures(black, red, blue, green, ufoTex);

    // HUD
    TTF_Font* font = am.font("assets/Bonus/kenvector_future.ttf", 16);
    SDL_Texture* lifeIcon = am.texture("assets/PNG/UI/playerLife1_orange.png");
    std::array<SDL_Texture*, 10> numerals;
    for (int i = 0; i < 10; ++i) {
        char buf[64];
        SDL_snprintf(buf, sizeof(buf), "assets/PNG/UI/numeral%d.png", i);
        numerals[i] = am.texture(buf);
    }
    m_hud = std::make_unique<HUD>(font, lifeIcon, numerals);

    // Collision callbacks
    m_collision.setCallbacks(
        [this](int pts){ m_score += pts; },
        [this](float x, float y){ spawnExplosion(x, y); }
    );

    m_menuBg = m_bgTextures[0];

    AudioManager::get().playMusic(
        "assets/music/05_Main Theme v1 (633 Squadron) (SID Stereo).mp3");

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

    if (input.isPressed(Action::PAUSE)) {
        if (m_state == GameState::PLAYING)
            m_state = GameState::PAUSED;
        else if (m_state == GameState::PAUSED)
            m_state = GameState::PLAYING;
    }

    if (m_state == GameState::MENU && input.isPressed(Action::CONFIRM)) {
        StageManager::get().reset();
        m_level = 1;
        m_waves.resetKillStats();
        m_state = GameState::PLAYING;
        m_waves.startWave(m_level);
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
        m_meteors.clear();
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
    m_player->update(dt);

    if (!m_player->isActive()) {
        StageManager::get().updateHighScore(m_score);
        m_state = GameState::GAMEOVER;
        AudioManager::get().playSound("assets/Bonus/sfx_lose.ogg");
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
        SDL_Texture* laserTex = AssetManager::get().texture(
            "assets/PNG/Lasers/laserBlue01.png");

        m_bullets.spawnPlayer(
            m_player->bounds().x + m_player->bounds().w * 0.5f - 5,
            m_player->bounds().y,
            m_player->fireLevel(), laserTex);

        // Wingman fire
        for (const auto& wm : m_player->wingmen()) {
            if (!wm.active) continue;
            m_bullets.spawnPlayer(
                m_player->bounds().x + m_player->bounds().w * 0.5f + wm.offsetX,
                m_player->bounds().y,
                1, laserTex);
        }

        AudioManager::get().playSound("assets/sounds/fire.mp3");
    }

    m_waves.update(dt);

    // Enemy firing (blocked when bullet-frozen)
    if (!m_player->isBulletFrozen()) {
        SDL_Texture* enemyLaser = AssetManager::get().texture(
            "assets/PNG/Lasers/laserRed01.png");
        for (auto& enemy : m_waves.enemies()) {
            if (!enemy->isActive()) continue;
            if (enemy->tryFire(dt)) {
                m_bullets.spawnEnemy(
                    enemy->bounds().x + enemy->bounds().w * 0.5f,
                    enemy->bounds().y + enemy->bounds().h,
                    0, ENEMY_BULLET_SPEED, enemyLaser);
            }
        }
    }

    m_bullets.update(dt);

    spawnMeteors(dt);
    for (auto& m : m_meteors) m->update(dt);

    for (auto& ex : m_explosions) ex.anim.update(dt);
    m_explosions.erase(
        std::remove_if(m_explosions.begin(), m_explosions.end(),
            [](const Explosion& ex){ return ex.anim.isFinished(); }),
        m_explosions.end());

    m_collision.check(*m_player, m_bullets, m_waves, m_meteors);

    m_meteors.erase(
        std::remove_if(m_meteors.begin(), m_meteors.end(),
            [](const auto& m){ return !m->isActive(); }),
        m_meteors.end());

    if (m_waves.isWaveCleared()) {
        m_stageKillCount    = m_waves.killCount();
        m_stageTotalEnemies = m_waves.totalSpawned();
        m_tallyTimer        = 0;
        m_tallyBonusAdded   = false;
        m_bullets.clear();
        AudioManager::get().playMusic(
            "assets/music/07_Stage Clear (SID Stereo) (1).mp3", 1);
        m_state = GameState::STAGE_TALLY;
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
    m_waves.render(m_renderer);
    m_bullets.render(m_renderer);
    for (auto& m : m_meteors) m->render(m_renderer);
    m_player->render(m_renderer);
    for (auto& ex : m_explosions)
        ex.anim.render(m_renderer, ex.x, ex.y, 64 * SPRITE_SCALE, 64 * SPRITE_SCALE);

    m_hud->render(m_renderer, m_score, m_player->lives(),
                  StageManager::get().currentStage(),
                  m_player->shieldLevel(),
                  m_player->loopsRemaining(),
                  StageManager::get().highScore());
}

void Game::renderMenu() {
    if (m_menuBg) {
        SDL_FRect dst = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
        SDL_RenderTexture(m_renderer, m_menuBg, nullptr, &dst);
    }
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 160);
    SDL_FRect box = {60, 220, 360, 160};
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
    SDL_FRect box = {40, 160, 400, 300};
    SDL_RenderFillRect(m_renderer, &box);

    int stage = StageManager::get().currentStage();
    std::string hdr = "STAGE " + std::to_string(stage) + " CLEAR";
    renderText(hdr, LOGICAL_W * 0.5f - (float)(hdr.size() * 5), 175, {255, 255, 100, 255}, 22);

    // Shooting %
    int pct = (m_stageTotalEnemies > 0)
              ? (m_stageKillCount * 100 / m_stageTotalEnemies) : 0;
    std::string pctTxt = "SHOOTING DOWN  " + std::to_string(pct) + "%";
    renderText(pctTxt, 70, 230, {255, 255, 255, 255}, 16);

    // Loop bonus
    int loopBonus = m_player->loopsRemaining() * SCORE_LOOP_BONUS;
    std::string loopTxt = "LOOP BONUS  " + std::to_string(m_player->loopsRemaining())
                        + " x 1000 = " + std::to_string(loopBonus);
    renderText(loopTxt, 70, 270, {255, 255, 255, 255}, 16);

    // Perfect bonus
    if (pct == 100)
        renderText("SPECIAL BONUS  50000", 70, 310, {255, 200, 0, 255}, 16);

    // Add bonuses exactly once
    if (!m_tallyBonusAdded) {
        m_tallyBonusAdded = true;
        m_score += loopBonus;
        if (pct == 100) m_score += SCORE_PERFECT_BONUS;
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
}

void Game::spawnMeteors(float dt) {
    m_meteorTimer += dt;
    if (m_meteorTimer < m_meteorInterval) return;
    m_meteorTimer = 0;

    static const char* bigMeteors[] = {
        "assets/PNG/Meteors/meteorBrown_big1.png",
        "assets/PNG/Meteors/meteorBrown_big2.png",
        "assets/PNG/Meteors/meteorGrey_big1.png",
        "assets/PNG/Meteors/meteorGrey_big2.png",
    };
    int idx  = std::rand() % 4;
    float vx = (std::rand() % 60) - 30.f;
    float vy = METEOR_SPEED_MIN + std::rand() % (int)(METEOR_SPEED_MAX - METEOR_SPEED_MIN);
    float x  = (float)(std::rand() % (LOGICAL_W - 80));

    SDL_Texture* tex = AssetManager::get().texture(bigMeteors[idx]);
    m_meteors.push_back(
        std::make_unique<Meteor>(x, -110.f, vx, vy, MeteorSize::BIG, tex));
}

void Game::advanceStage() {
    // Add loop bonus and reset tally state to avoid double-counting
    // (bonus already added in renderStageTally on first render, so only add here)
    // Note: to avoid double-adding, we track that separately. For simplicity,
    // loop bonus and perfect bonus are only added once in renderStageTally.

    StageManager::get().advance();
    ++m_level;
    m_meteorInterval = std::max(1.0f, m_meteorInterval - 0.2f);

    // Switch background for new stage
    int bgIdx = StageManager::get().currentDef().bgIndex;
    m_background->setTexture(m_bgTextures[bgIdx]);

    m_player->resetForNewStage();
    m_waves.resetKillStats();
    m_waves.startWave(m_level);

    AudioManager::get().playMusic(
        "assets/music/05_Main Theme v1 (633 Squadron) (SID Stereo).mp3");

    m_state = GameState::PLAYING;
}

void Game::resetPlayer() {
    m_score = 0;
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
    m_background.reset();
    m_hud.reset();
    m_waves.clear();
    m_bullets.clear();
    m_meteors.clear();
    m_explosions.clear();
    m_explosionFrames.clear();

    AudioManager::get().shutdown();
    AssetManager::get().shutdown();

    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }

    TTF_Quit();
    SDL_Quit();
}
