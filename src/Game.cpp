#include "Game.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "InputManager.h"
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

    m_window = SDL_CreateWindow("1942 Shooter",
                                 480, 640,
                                 SDL_WINDOW_RESIZABLE);
    if (!m_window) { SDL_Log("CreateWindow: %s", SDL_GetError()); return false; }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) { SDL_Log("CreateRenderer: %s", SDL_GetError()); return false; }

    SDL_SetRenderLogicalPresentation(m_renderer,
                                     LOGICAL_W, LOGICAL_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Base path ends with separator
    std::string base = SDL_GetBasePath();

    AssetManager::get().init(m_renderer, base);
    AudioManager::get().init(base);

    // ---- Load all assets ----
    auto& am = AssetManager::get();

    // Background
    m_background = std::make_unique<Background>(
        am.texture("assets/Backgrounds/darkPurple.png"), 70.f);

    // Explosion frames fire00-19
    m_explosionFrames.reserve(20);
    for (int i = 0; i < 20; ++i) {
        char buf[64];
        SDL_snprintf(buf, sizeof(buf), "assets/PNG/Effects/fire%02d.png", i);
        m_explosionFrames.push_back(am.texture(buf));
    }

    // Player
    SDL_Texture* shipTex = am.texture("assets/PNG/playerShip1_orange.png");
    std::array<SDL_Texture*, 3> damageTex = {
        am.texture("assets/PNG/Damage/playerShip1_damage1.png"),
        am.texture("assets/PNG/Damage/playerShip1_damage2.png"),
        am.texture("assets/PNG/Damage/playerShip1_damage3.png"),
    };
    resetPlayer();
    m_player = std::make_unique<Player>(
        LOGICAL_W * 0.5f - 33, LOGICAL_H - 120.f,
        shipTex, damageTex);

    // Enemy textures
    SDL_Texture* black[5], *red[5], *blue[5], *green[5];
    for (int i = 0; i < 5; ++i) {
        char buf[64];
        SDL_snprintf(buf,sizeof(buf),"assets/PNG/Enemies/enemyBlack%d.png",i+1);
        black[i] = am.texture(buf);
        SDL_snprintf(buf,sizeof(buf),"assets/PNG/Enemies/enemyRed%d.png",i+1);
        red[i]   = am.texture(buf);
        SDL_snprintf(buf,sizeof(buf),"assets/PNG/Enemies/enemyBlue%d.png",i+1);
        blue[i]  = am.texture(buf);
        SDL_snprintf(buf,sizeof(buf),"assets/PNG/Enemies/enemyGreen%d.png",i+1);
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

    // Menu background
    m_menuBg = am.texture("assets/Backgrounds/darkPurple.png");

    // Start music
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

    // Global pause toggle
    if (input.isPressed(Action::PAUSE)) {
        if (m_state == GameState::PLAYING)
            m_state = GameState::PAUSED;
        else if (m_state == GameState::PAUSED)
            m_state = GameState::PLAYING;
    }

    if (m_state == GameState::MENU && input.isPressed(Action::CONFIRM)) {
        m_state = GameState::PLAYING;
        m_waves.startWave(m_level);
    }

    if (m_state == GameState::GAMEOVER && input.isPressed(Action::CONFIRM)) {
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
    if (m_state != GameState::PLAYING) return;
    updatePlaying(dt);
}

void Game::updatePlaying(float dt) {
    m_background->update(dt);

    m_player->update(dt);
    if (!m_player->isActive()) {
        m_state = GameState::GAMEOVER;
        AudioManager::get().playSound("assets/Bonus/sfx_lose.ogg");
        return;
    }

    // Player firing
    if (m_player->wantsToFire()) {
        SDL_Texture* laserTex = AssetManager::get().texture(
            "assets/PNG/Lasers/laserBlue01.png");
        m_bullets.spawnPlayer(
            m_player->bounds().x + m_player->bounds().w * 0.5f - 5,
            m_player->bounds().y,
            m_player->fireLevel(), laserTex);
        AudioManager::get().playSound("assets/sounds/fire.mp3");
    }

    m_waves.update(dt);

    // Enemy firing
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

    m_bullets.update(dt);

    spawnMeteors(dt);
    for (auto& m : m_meteors) m->update(dt);

    // Explosions
    for (auto& ex : m_explosions) ex.anim.update(dt);
    m_explosions.erase(
        std::remove_if(m_explosions.begin(), m_explosions.end(),
            [](const Explosion& ex){ return ex.anim.isFinished(); }),
        m_explosions.end());

    m_collision.check(*m_player, m_bullets, m_waves, m_meteors);

    // Clean up off-screen meteors
    m_meteors.erase(
        std::remove_if(m_meteors.begin(), m_meteors.end(),
            [](const auto& m){ return !m->isActive(); }),
        m_meteors.end());

    // Next wave
    if (m_waves.isWaveCleared()) {
        ++m_level;
        m_meteorInterval = std::max(1.0f, m_meteorInterval - 0.2f);
        nextWave();
    }
}

// -----------------------------------------------------------------------
void Game::render() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    switch (m_state) {
    case GameState::MENU:     renderMenu();     break;
    case GameState::PLAYING:  renderPlaying();  break;
    case GameState::PAUSED:   renderPaused();   break;
    case GameState::GAMEOVER: renderGameOver(); break;
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
        ex.anim.render(m_renderer, ex.x, ex.y, 64, 64);
    m_hud->render(m_renderer, m_score, m_player->lives(),
                  m_level, m_player->shieldLevel());
}

void Game::renderMenu() {
    if (m_menuBg) {
        SDL_FRect dst = {0,0,(float)LOGICAL_W,(float)LOGICAL_H};
        SDL_RenderTexture(m_renderer, m_menuBg, nullptr, &dst);
    }
    // Title text via HUD font
    // Simple approach: render coloured rectangle + text
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 160);
    SDL_FRect box = {60, 220, 360, 160};
    SDL_RenderFillRect(m_renderer, &box);

    // Use TTF to draw title (quick inline render)
    TTF_Font* f = AssetManager::get().font(
        "assets/Bonus/kenvector_future.ttf", 28);
    if (f) {
        SDL_Color white = {255,255,255,255};
        SDL_Surface* s = TTF_RenderText_Blended(f,"1942 SHOOTER",12,white);
        if (s) {
            SDL_Texture* t = SDL_CreateTextureFromSurface(m_renderer, s);
            SDL_DestroySurface(s);
            if (t) {
                float tw,th;
                SDL_GetTextureSize(t,&tw,&th);
                SDL_FRect d = {LOGICAL_W*0.5f-tw*0.5f, 240, tw, th};
                SDL_RenderTexture(m_renderer,t,nullptr,&d);
                SDL_DestroyTexture(t);
            }
        }
        SDL_Color yellow = {255,220,0,255};
        SDL_Surface* s2 = TTF_RenderText_Blended(f,"PRESS ENTER TO PLAY",19,yellow);
        if (s2) {
            SDL_Texture* t2 = SDL_CreateTextureFromSurface(m_renderer, s2);
            SDL_DestroySurface(s2);
            if (t2) {
                float tw,th;
                SDL_GetTextureSize(t2,&tw,&th);
                SDL_FRect d = {LOGICAL_W*0.5f-tw*0.5f, 310, tw, th};
                SDL_RenderTexture(m_renderer,t2,nullptr,&d);
                SDL_DestroyTexture(t2);
            }
        }
    }
}

void Game::renderPaused() {
    renderPlaying();
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 120);
    SDL_FRect overlay = {0, 0, (float)LOGICAL_W, (float)LOGICAL_H};
    SDL_RenderFillRect(m_renderer, &overlay);
}

void Game::renderGameOver() {
    if (m_menuBg) {
        SDL_FRect dst = {0,0,(float)LOGICAL_W,(float)LOGICAL_H};
        SDL_RenderTexture(m_renderer, m_menuBg, nullptr, &dst);
    }
    TTF_Font* f = AssetManager::get().font(
        "assets/Bonus/kenvector_future.ttf", 28);
    if (f) {
        SDL_Color red = {255,60,60,255};
        SDL_Surface* s = TTF_RenderText_Blended(f,"GAME OVER",9,red);
        if (s) {
            SDL_Texture* t = SDL_CreateTextureFromSurface(m_renderer,s);
            SDL_DestroySurface(s);
            if (t) {
                float tw,th; SDL_GetTextureSize(t,&tw,&th);
                SDL_FRect d = {LOGICAL_W*0.5f-tw*0.5f,260,tw,th};
                SDL_RenderTexture(m_renderer,t,nullptr,&d);
                SDL_DestroyTexture(t);
            }
        }
        SDL_Color white = {255,255,255,255};
        std::string sc = "SCORE: " + std::to_string(m_score);
        SDL_Surface* s2 = TTF_RenderText_Blended(f,sc.c_str(),
                                                  sc.size(),white);
        if (s2) {
            SDL_Texture* t2 = SDL_CreateTextureFromSurface(m_renderer,s2);
            SDL_DestroySurface(s2);
            if (t2) {
                float tw,th; SDL_GetTextureSize(t2,&tw,&th);
                SDL_FRect d = {LOGICAL_W*0.5f-tw*0.5f,310,tw,th};
                SDL_RenderTexture(m_renderer,t2,nullptr,&d);
                SDL_DestroyTexture(t2);
            }
        }
    }
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

void Game::nextWave() {
    m_bullets.clear();
    m_waves.startWave(m_level);
    AudioManager::get().playMusic(
        "assets/music/07_Stage Clear (SID Stereo) (1).mp3", 1);
    // Resume main theme after stage clear plays
    // (simple: just restart main theme)
    AudioManager::get().playMusic(
        "assets/music/05_Main Theme v1 (633 Squadron) (SID Stereo).mp3");
}

void Game::resetPlayer() {
    // Called before m_player is constructed or on game-over restart.
    // Actual player reset happens in init() / game-over handler by
    // creating a new Player. Here we just reset state flags used before
    // player creation.
    m_score = 0;
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
