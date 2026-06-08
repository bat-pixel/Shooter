#include "LevelData.h"

// World Y scripts per area. worldY = distance flown in pixels (80px/s background).
// A stage lasts roughly 30-60 seconds = 2400-4800 world units.
// Scripts loop when the stage outlasts them.

static const std::vector<LevelObject> s_area0 = {
    { TerrainType::SMALL_ISLAND,   900,  70 },
    { TerrainType::SMALL_ISLAND,  1600, 270 },
    { TerrainType::BIG_ISLAND,    2400, 160 },
    { TerrainType::SMALL_ISLAND,  3100,  50 },
    { TerrainType::CARRIER_SHIP,  3800, 200 },
    { TerrainType::SMALL_ISLAND,  4600, 290 },
    { TerrainType::BIG_ISLAND,    5400, 100 },
};

static const std::vector<LevelObject> s_area1 = {
    { TerrainType::SMALL_ISLAND,   700,  80 },
    { TerrainType::BIG_ISLAND,    1400, 240 },
    { TerrainType::SMALL_ISLAND,  2000, 300 },
    { TerrainType::CARRIER_SHIP,  2700, 160 },
    { TerrainType::SMALL_ISLAND,  3300,  60 },
    { TerrainType::BIG_ISLAND,    3900, 200 },
    { TerrainType::CARRIER_SHIP,  4800, 100 },
    { TerrainType::SMALL_ISLAND,  5400, 280 },
};

static const std::vector<LevelObject> s_area2 = {
    { TerrainType::SMALL_ISLAND,   600,  90 },
    { TerrainType::CARRIER_SHIP,  1200, 180 },
    { TerrainType::BIG_ISLAND,    1800, 260 },
    { TerrainType::SMALL_ISLAND,  2300,  40 },
    { TerrainType::CARRIER_SHIP,  3000, 300 },
    { TerrainType::BIG_ISLAND,    3600, 130 },
    { TerrainType::SMALL_ISLAND,  4100, 240 },
    { TerrainType::CARRIER_SHIP,  4700,  80 },
    { TerrainType::BIG_ISLAND,    5300, 200 },
};

static const std::vector<LevelObject> s_area3 = {
    { TerrainType::CARRIER_SHIP,   500, 160 },
    { TerrainType::SMALL_ISLAND,  1000,  60 },
    { TerrainType::BIG_ISLAND,    1500, 250 },
    { TerrainType::CARRIER_SHIP,  2100, 100 },
    { TerrainType::SMALL_ISLAND,  2600, 290 },
    { TerrainType::CARRIER_SHIP,  3200, 200 },
    { TerrainType::BIG_ISLAND,    3700,  70 },
    { TerrainType::CARRIER_SHIP,  4300, 150 },
    { TerrainType::SMALL_ISLAND,  4800, 280 },
    { TerrainType::CARRIER_SHIP,  5300, 100 },
};

const std::vector<LevelObject>& getLevelScript(int area) {
    switch (area) {
    case 1:  return s_area1;
    case 2:  return s_area2;
    case 3:  return s_area3;
    default: return s_area0;
    }
}
