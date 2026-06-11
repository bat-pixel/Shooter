#include "Game.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
static Game* g_game = nullptr;
static void em_main_loop() { g_game->stepFrame(); }
#endif

int main(int /*argc*/, char* /*argv*/[]) {
    static Game game;
    if (!game.init()) {
        SDL_Log("Failed to initialise game");
        return 1;
    }
#ifdef __EMSCRIPTEN__
    g_game = &game;
    // 0 fps = match display refresh rate; 1 = simulate infinite loop (blocks)
    emscripten_set_main_loop(em_main_loop, 0, 1);
    // shutdown() is called from stepFrame() when m_running goes false
#else
    game.run();
    game.shutdown();
#endif
    return 0;
}
