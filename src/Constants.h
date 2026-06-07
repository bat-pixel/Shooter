#pragma once
#include <string>

constexpr float SPRITE_SCALE = 0.25f;

constexpr int   LOGICAL_W    = 480;
constexpr int   LOGICAL_H    = 640;
constexpr int   TARGET_FPS   = 60;
constexpr float MAX_DT       = 0.05f;   // cap to avoid spiral-of-death

// Asset root (resolved at runtime via SDL_GetBasePath)
// All paths are relative to this.

// ---- Player ----
constexpr float PLAYER_SPEED       = 220.0f;  // px/s
constexpr float PLAYER_BULLET_SPEED= 500.0f;
constexpr float PLAYER_FIRE_RATE   = 0.18f;   // seconds between shots
constexpr float LOOP_DURATION      = 1.2f;    // invincibility window (s)
constexpr int   PLAYER_MAX_LIVES   = 3;

// ---- Enemies ----
constexpr float ENEMY_SPEED_BASE   = 80.0f;
constexpr float ENEMY_BULLET_SPEED = 250.0f;
constexpr float ENEMY_FIRE_RATE    = 2.0f;

// ---- Meteors ----
constexpr float METEOR_SPEED_MIN   = 60.0f;
constexpr float METEOR_SPEED_MAX   = 160.0f;

// ---- Scoring ----
constexpr int SCORE_ENEMY_SMALL    = 100;
constexpr int SCORE_ENEMY_MEDIUM   = 200;
constexpr int SCORE_METEOR         = 50;
constexpr int SCORE_UFO            = 500;

// ---- Bullet pool ----
constexpr int BULLET_POOL_SIZE     = 128;

// ---- Stage system ----
constexpr int STAGE_COUNT        = 32;
constexpr int PLAYER_START_LOOPS = 3;

// ---- Scoring bonuses ----
constexpr int SCORE_FORMATION_5   = 500;
constexpr int SCORE_FORMATION_10  = 1000;
constexpr int SCORE_LOOP_BONUS    = 1000;
constexpr int SCORE_PERFECT_BONUS = 50000;
constexpr int SCORE_POW_RED       = 1000;
constexpr int SCORE_YASHICHI      = 5000;
constexpr int SCORE_BOSS          = 10000;

// ---- Enemy HP ----
constexpr int ENEMY_HP_SMALL  = 1;
constexpr int ENEMY_HP_MEDIUM = 3;
constexpr int ENEMY_HP_LARGE  = 8;
constexpr int BOSS_HP         = 60;
