#include "LevelObjectManager.h"
#include "../Constants.h"
#include <algorithm>

void LevelObjectManager::startStage(int area,
                                     SDL_Texture* texSmall,
                                     SDL_Texture* texBig,
                                     SDL_Texture* texCarrier) {
    m_objects.clear();
    m_scriptIdx  = 0;
    m_loopOffset = 0;
    m_texSmall   = texSmall;
    m_texBig     = texBig;
    m_texCarrier = texCarrier;

    m_script = &getLevelScript(area);

    if (!m_script->empty())
        m_loopSpan = m_script->back().worldY + 1000.f;
    else
        m_loopSpan = 5000.f;
}

void LevelObjectManager::update(float worldY, float dt) {
    // Update existing objects
    for (auto& t : m_objects) t->update(dt);

    m_objects.erase(
        std::remove_if(m_objects.begin(), m_objects.end(),
            [](const auto& t){ return !t->isActive(); }),
        m_objects.end());

    if (!m_script || m_script->empty()) return;

    int n = (int)m_script->size();
    while (m_scriptIdx < n) {
        float threshold = (*m_script)[m_scriptIdx % n].worldY + m_loopOffset;
        if (worldY < threshold) break;

        const LevelObject& obj = (*m_script)[m_scriptIdx % n];
        SDL_Texture* tex = nullptr;
        int hp = 0;

        switch (obj.type) {
        case TerrainType::SMALL_ISLAND:  tex = m_texSmall;   hp = 0;  break;
        case TerrainType::BIG_ISLAND:    tex = m_texBig;     hp = 0;  break;
        case TerrainType::CARRIER_SHIP:  tex = m_texCarrier; hp = 20; break;
        }

        if (tex)
            m_objects.push_back(std::make_unique<Terrain>(tex, obj.x, 80.f, hp));

        ++m_scriptIdx;

        // Loop the script when exhausted
        if (m_scriptIdx >= n) {
            m_scriptIdx  = 0;
            m_loopOffset += m_loopSpan;
        }
    }
}

void LevelObjectManager::render(SDL_Renderer* renderer) {
    for (auto& t : m_objects) t->render(renderer);
}

void LevelObjectManager::clear() {
    m_objects.clear();
    m_scriptIdx  = 0;
    m_loopOffset = 0;
    m_script     = nullptr;
}
