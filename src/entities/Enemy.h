#pragma once
#include "BaseEntity.h"
#include <SDL3/SDL.h>
#include <functional>

enum class EnemyType { SMALL, MEDIUM, LARGE, UFO };
enum class EnemyPattern { STRAIGHT, SINE, DIVE };

class Enemy : public BaseEntity {
public:
    Enemy(float x, float y, EnemyType type, EnemyPattern pattern,
          SDL_Texture* tex, SDL_Texture* damageTex = nullptr);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

    EnemyType type() const { return m_type; }
    int       scoreValue() const;

    bool tryFire(float dt);   // returns true when enemy fires; caller spawns bullet

    // Formation offset so WaveManager can set target position
    void setFormationTarget(float tx, float ty) { m_targetX = tx; m_targetY = ty; }

private:
    SDL_Texture*  m_tex;
    SDL_Texture*  m_damageTex;
    EnemyType     m_type;
    EnemyPattern  m_pattern;
    float         m_time        = 0;
    float         m_fireTimer   = 0;
    float         m_fireRate;
    float         m_targetX     = 0;
    float         m_targetY     = 0;
    bool          m_inFormation = false;
};
