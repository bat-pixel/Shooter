#include "SpriteAnimation.h"
#include "../Constants.h"

SpriteAnimation::SpriteAnimation(std::vector<SDL_Texture*> frames,
                                 float fps, bool loop)
    : m_frames(std::move(frames)), m_fps(fps), m_loop(loop) {}

void SpriteAnimation::update(float dt) {
    if (m_finished) return;
    m_elapsed += dt;
    float frameDur = 1.0f / m_fps;
    while (m_elapsed >= frameDur) {
        m_elapsed -= frameDur;
        ++m_current;
        if (m_current >= (int)m_frames.size()) {
            if (m_loop) {
                m_current = 0;
            } else {
                m_current  = (int)m_frames.size() - 1;
                m_finished = true;
                return;
            }
        }
    }
}

void SpriteAnimation::render(SDL_Renderer* renderer,
                             float x, float y,
                             float w, float h, double angle) {
    if (m_frames.empty()) return;
    SDL_Texture* tex = m_frames[m_current];
    if (!tex) return;

    float tw = w, th = h;
    if (tw == 0 || th == 0) {
        SDL_GetTextureSize(tex, &tw, &th);
        tw *= SPRITE_SCALE; th *= SPRITE_SCALE;
    }
    SDL_FRect dst = {x, y, tw, th};
    if (angle != 0.0)
        SDL_RenderTextureRotated(renderer, tex, nullptr, &dst,
                                 angle, nullptr, SDL_FLIP_NONE);
    else
        SDL_RenderTexture(renderer, tex, nullptr, &dst);
}

bool SpriteAnimation::isFinished() const { return m_finished; }

void SpriteAnimation::reset() {
    m_current  = 0;
    m_elapsed  = 0;
    m_finished = false;
}
