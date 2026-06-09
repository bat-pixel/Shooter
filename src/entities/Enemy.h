#pragma once
#include "BaseEntity.h"
#include <SDL3/SDL.h>

enum class EnemyType { SMALL, MEDIUM, LARGE, UFO };
enum class EnemyPattern { STRAIGHT, SINE, DIVE, ARC, LOOP_DIVE };

class Enemy : public BaseEntity {
public:
    Enemy(float x, float y, EnemyType type, EnemyPattern pattern,
          SDL_Texture* tex, SDL_Texture* damageTex = nullptr,
          float speedMult = 1.f);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    EnemyType type()       const { return m_type; }
    int       scoreValue() const;
    int       hp()         const { return m_hp; }
    int       maxHp()      const { return m_maxHp; }

    bool tryFire(float dt);
    bool hit();  // decrements HP; returns true when the enemy dies

    void setFormationTarget(float tx, float ty) { m_targetX = tx; m_targetY = ty; }
    void setPlayerTarget(float px, float py)   { m_playerX = px; m_playerY = py; }
    void setBaseAngle(float a)                 { m_baseAngle = a; }

private:
    SDL_Texture* m_tex;
    SDL_Texture* m_damageTex;
    EnemyType    m_type;
    EnemyPattern m_pattern;
    float        m_time          = 0;
    float        m_fireTimer     = 0;
    float        m_fireRate;
    float        m_initialX      = 0;   // SINE: anchor column
    float        m_targetX       = 0;
    float        m_targetY       = 0;
    bool         m_inFormation   = false;
    float        m_formationTimer = 0;  // DIVE: time spent in formation
    float        m_loopAngle     = 0;   // DIVE: current angle on loop circle
    float        m_loopCenterX   = 0;
    float        m_loopCenterY   = 0;
    bool         m_looping       = false; // DIVE: in circular loop phase
    bool         m_diving        = false; // DIVE: post-loop straight dive
    // ARC / LOOP_DIVE state
    int          m_phase        = 0;     // generic phase counter
    float        m_phaseTimer   = 0.f;   // timer within phase
    float        m_arcForce     = 1.f;   // ARC: curve acceleration direction (+1/-1)
    float        m_diveVelX     = 0.f;   // LOOP_DIVE: aimed dive velocity X
    float        m_diveVelY     = 0.f;   // LOOP_DIVE: aimed dive velocity Y
    float        m_playerX      = 180.f; // player center X (updated each frame)
    float        m_playerY      = 320.f; // player center Y
    float        m_renderAngle  = 0.f;   // physics-derived rotation in degrees
    float        m_baseAngle   = 0.f;   // sprite orientation offset (180° for south-facing assets)
    int          m_hp            = 1;
    int          m_maxHp         = 1;
};
