#include "LevelData.h"

// World Y scripts per area. worldY = distance flown in pixels (80px/s background).
// A stage lasts roughly 30-60 seconds = 2400-4800 world units.
// Scripts loop when the stage outlasts them.

static const std::vector<LevelObject> s_area0 = {
    { TerrainType::SMALL_ISLAND,   300,  70 },
    { TerrainType::SMALL_ISLAND,   900, 270 },
    { TerrainType::BIG_ISLAND,    1600, 160 },
    { TerrainType::SMALL_ISLAND,  2200,  50 },
    { TerrainType::CARRIER_SHIP,  2900, 200 },
    { TerrainType::SMALL_ISLAND,  3600, 290 },
    { TerrainType::BIG_ISLAND,    4300, 100 },
    { TerrainType::CARRIER_SHIP,  5000, 170 },
};

static const std::vector<LevelObject> s_area1 = {
    { TerrainType::SMALL_ISLAND,   250,  80 },
    { TerrainType::BIG_ISLAND,     900, 240 },
    { TerrainType::SMALL_ISLAND,  1500, 300 },
    { TerrainType::CARRIER_SHIP,  2100, 160 },
    { TerrainType::SMALL_ISLAND,  2700,  60 },
    { TerrainType::BIG_ISLAND,    3300, 200 },
    { TerrainType::CARRIER_SHIP,  3900, 100 },
    { TerrainType::SMALL_ISLAND,  4500, 280 },
};

static const std::vector<LevelObject> s_area2 = {
    { TerrainType::SMALL_ISLAND,   200,  90 },
    { TerrainType::CARRIER_SHIP,   800, 180 },
    { TerrainType::BIG_ISLAND,    1400, 260 },
    { TerrainType::SMALL_ISLAND,  1900,  40 },
    { TerrainType::CARRIER_SHIP,  2500, 300 },
    { TerrainType::BIG_ISLAND,    3100, 130 },
    { TerrainType::SMALL_ISLAND,  3600, 240 },
    { TerrainType::CARRIER_SHIP,  4200,  80 },
    { TerrainType::BIG_ISLAND,    4800, 200 },
};

static const std::vector<LevelObject> s_area3 = {
    { TerrainType::CARRIER_SHIP,   200, 160 },
    { TerrainType::SMALL_ISLAND,   700,  60 },
    { TerrainType::BIG_ISLAND,    1200, 250 },
    { TerrainType::CARRIER_SHIP,  1700, 100 },
    { TerrainType::SMALL_ISLAND,  2200, 290 },
    { TerrainType::CARRIER_SHIP,  2700, 200 },
    { TerrainType::BIG_ISLAND,    3200,  70 },
    { TerrainType::CARRIER_SHIP,  3700, 150 },
    { TerrainType::SMALL_ISLAND,  4200, 280 },
    { TerrainType::CARRIER_SHIP,  4700, 100 },
};

const std::vector<LevelObject>& getLevelScript(int area) {
    switch (area) {
    case 1:  return s_area1;
    case 2:  return s_area2;
    case 3:  return s_area3;
    default: return s_area0;
    }
}
