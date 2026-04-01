#pragma once
#include <SDL3/SDL.h>
#include <vector>

class SpriteAnimation {
public:
    // frames: array of textures (one per frame), fps: playback speed
    SpriteAnimation(std::vector<SDL_Texture*> frames, float fps, bool loop = true);

    void update(float dt);
    void render(SDL_Renderer* renderer, float x, float y,
                float w = 0, float h = 0, double angle = 0.0);

    bool isFinished() const;   // only meaningful when loop=false
    void reset();

private:
    std::vector<SDL_Texture*> m_frames;
    float m_fps;
    bool  m_loop;
    float m_elapsed   = 0;
    int   m_current   = 0;
    bool  m_finished  = false;
};
