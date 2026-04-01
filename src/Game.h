#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <memory>

#include "entities/Player.h"
#include "entities/Meteor.h"
#include "managers/BulletManager.h"
#include "managers/EnemyWaveManager.h"
#include "managers/CollisionManager.h"
#include "rendering/Background.h"
#include "rendering/SpriteAnimation.h"
#include "rendering/HUD.h"

enum class GameState { MENU, PLAYING, PAUSED, GAMEOVER };

struct Explosion {
    SpriteAnimation anim;
    float x, y;
};

class Game {
public:
    bool init();
    void run();
    void shutdown();

private:
    void handleEvents();
    void update(float dt);
    void render();

    void updatePlaying(float dt);
    void renderMenu();
    void renderPlaying();
    void renderPaused();
    void renderGameOver();

    void spawnExplosion(float x, float y);
    void spawnMeteors(float dt);
    void nextWave();
    void resetPlayer();

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool          m_running  = false;

    GameState m_state = GameState::MENU;

    int m_score = 0;
    int m_level = 1;

    // Entities / managers
    std::unique_ptr<Player>         m_player;
    std::unique_ptr<Background>     m_background;
    std::unique_ptr<HUD>            m_hud;
    BulletManager                   m_bullets;
    EnemyWaveManager                m_waves;
    CollisionManager                m_collision;

    std::vector<std::unique_ptr<Meteor>>    m_meteors;
    std::vector<Explosion>                  m_explosions;

    float m_meteorTimer   = 0;
    float m_meteorInterval= 3.0f;

    // Explosion frames (fire00-19)
    std::vector<SDL_Texture*> m_explosionFrames;

    // Menu / UI textures
    SDL_Texture* m_menuBg = nullptr;
};
