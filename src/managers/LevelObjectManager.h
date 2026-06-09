#pragma once
#include "../entities/Terrain.h"
#include "../data/LevelData.h"
#include <vector>
#include <memory>
#include <SDL3/SDL.h>

class LevelObjectManager {
public:
    void startStage(int area,
                    const std::vector<SDL_Texture*>& smallVariants,
                    SDL_Texture* texBig,
                    SDL_Texture* texCarrier);
    void update(float worldY, float dt);
    void render(SDL_Renderer* renderer);
    void clear();

    std::vector<std::unique_ptr<Terrain>>& objects() { return m_objects; }

private:
    std::vector<std::unique_ptr<Terrain>> m_objects;
    const std::vector<LevelObject>*       m_script         = nullptr;
    int                                   m_scriptIdx      = 0;
    float                                 m_loopOffset     = 0;
    float                                 m_loopSpan       = 0;
    int                                   m_smallVariantIdx = 0;

    std::vector<SDL_Texture*> m_smallVariants;
    SDL_Texture*              m_texBig     = nullptr;
    SDL_Texture*              m_texCarrier = nullptr;
};
