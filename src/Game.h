#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <memory>

#include "entities/Player.h"
#include "entities/Boss.h"
#include "managers/BulletManager.h"
#include "managers/EnemyWaveManager.h"
#include "managers/CollisionManager.h"
#include "managers/LevelObjectManager.h"
#include "rendering/Background.h"
#include "rendering/SpriteAnimation.h"
#include "rendering/HUD.h"

enum class GameState { MENU, PLAYING, PAUSED, STAGE_TALLY, GAMEOVER };

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
    void updateStageTally(float dt);
    void renderMenu();
    void renderPlaying();
    void renderPaused();
    void renderStageTally();
    void renderGameOver();

    void spawnExplosion(float x, float y);
    void spawnBoss();
    void advanceStage();
    void resetPlayer();
    void renderText(const std::string& text, float x, float y,
                    SDL_Color col, int ptSize);

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool          m_running  = false;

    GameState m_state = GameState::MENU;

    int m_score = 0;
    int m_level = 1;

    float m_tallyTimer        = 0;
    int   m_stageKillCount    = 0;
    int   m_stageTotalEnemies = 0;
    bool  m_tallyBonusAdded   = false;

    std::unique_ptr<Player>     m_player;
    std::unique_ptr<Boss>       m_boss;
    std::unique_ptr<Background> m_background;
    std::unique_ptr<HUD>        m_hud;
    BulletManager               m_bullets;
    EnemyWaveManager            m_waves;
    CollisionManager            m_collision;

    bool m_bossSpawned = false;

    std::vector<Explosion>   m_explosions;
    std::vector<SDL_Texture*> m_explosionFrames;

    LevelObjectManager m_levelObjects;
    float              m_worldY = 0;

    SDL_Texture* m_menuBg           = nullptr;
    SDL_Texture* m_playerBulletTex  = nullptr;
    SDL_Texture* m_enemyBulletTex   = nullptr;
    SDL_Texture* m_bossTex          = nullptr;
    std::vector<SDL_Texture*> m_terrainSmallTextures;
    SDL_Texture* m_terrainBigTex    = nullptr;
    SDL_Texture* m_terrainCarrierTex = nullptr;
};
