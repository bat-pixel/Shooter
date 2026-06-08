#include "LevelData.h"

// World Y scripts per area. worldY = distance flown in pixels (80px/s background).
// A stage lasts roughly 30-60 seconds = 2400-4800 world units.
// Scripts loop when the stage outlasts them.

// worldY at 80px/s: 240=3s, 480=6s, 800=10s, 1200=15s, 1600=20s
static const std::vector<LevelObject> s_area0 = {
    { TerrainType::SMALL_ISLAND,   240,  60 },
    { TerrainType::BIG_ISLAND,     600, 200 },
    { TerrainType::SMALL_ISLAND,   960, 280 },
    { TerrainType::CARRIER_SHIP,  1280, 160 },
    { TerrainType::BIG_ISLAND,    1680,  80 },
    { TerrainType::SMALL_ISLAND,  2080, 300 },
    { TerrainType::CARRIER_SHIP,  2480, 140 },
};

static const std::vector<LevelObject> s_area1 = {
    { TerrainType::SMALL_ISLAND,   200,  80 },
    { TerrainType::CARRIER_SHIP,   560, 220 },
    { TerrainType::BIG_ISLAND,     880, 120 },
    { TerrainType::SMALL_ISLAND,  1200, 290 },
    { TerrainType::CARRIER_SHIP,  1520, 160 },
    { TerrainType::BIG_ISLAND,    1840,  60 },
    { TerrainType::CARRIER_SHIP,  2160, 240 },
};

static const std::vector<LevelObject> s_area2 = {
    { TerrainType::BIG_ISLAND,     200, 160 },
    { TerrainType::CARRIER_SHIP,   480, 280 },
    { TerrainType::SMALL_ISLAND,   800,  60 },
    { TerrainType::CARRIER_SHIP,  1120, 200 },
    { TerrainType::BIG_ISLAND,    1440, 100 },
    { TerrainType::CARRIER_SHIP,  1760, 260 },
    { TerrainType::BIG_ISLAND,    2080,  80 },
    { TerrainType::CARRIER_SHIP,  2400, 180 },
};

static const std::vector<LevelObject> s_area3 = {
    { TerrainType::CARRIER_SHIP,   160, 200 },
    { TerrainType::BIG_ISLAND,     400,  80 },
    { TerrainType::CARRIER_SHIP,   720, 280 },
    { TerrainType::BIG_ISLAND,    1040, 140 },
    { TerrainType::CARRIER_SHIP,  1280, 220 },
    { TerrainType::BIG_ISLAND,    1600,  60 },
    { TerrainType::CARRIER_SHIP,  1920, 300 },
    { TerrainType::BIG_ISLAND,    2240, 160 },
    { TerrainType::CARRIER_SHIP,  2560, 100 },
};

const std::vector<LevelObject>& getLevelScript(int area) {
    switch (area) {
    case 1:  return s_area1;
    case 2:  return s_area2;
    case 3:  return s_area3;
    default: return s_area0;
    }
}
