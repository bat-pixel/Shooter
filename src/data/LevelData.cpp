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

// Area 4 — Leyte: dense carrier groups flanked by islands
static const std::vector<LevelObject> s_area4 = {
    { TerrainType::CARRIER_SHIP,   160, 180 },
    { TerrainType::CARRIER_SHIP,   400, 270 },
    { TerrainType::BIG_ISLAND,     640,  60 },
    { TerrainType::CARRIER_SHIP,   880, 200 },
    { TerrainType::SMALL_ISLAND,  1120, 300 },
    { TerrainType::CARRIER_SHIP,  1360, 140 },
    { TerrainType::CARRIER_SHIP,  1600, 250 },
    { TerrainType::BIG_ISLAND,    1840, 100 },
    { TerrainType::CARRIER_SHIP,  2080, 210 },
};

// Area 5 — Saipan: industrial port, few islands, many carriers
static const std::vector<LevelObject> s_area5 = {
    { TerrainType::CARRIER_SHIP,   120, 220 },
    { TerrainType::CARRIER_SHIP,   320, 100 },
    { TerrainType::CARRIER_SHIP,   520, 260 },
    { TerrainType::SMALL_ISLAND,   720,  80 },
    { TerrainType::CARRIER_SHIP,   960, 180 },
    { TerrainType::CARRIER_SHIP,  1200, 280 },
    { TerrainType::CARRIER_SHIP,  1440, 120 },
    { TerrainType::BIG_ISLAND,    1680,  60 },
    { TerrainType::CARRIER_SHIP,  1920, 200 },
    { TerrainType::CARRIER_SHIP,  2160, 300 },
};

// Area 6 — Iwo Jima: volcanic island, heavy carrier presence
static const std::vector<LevelObject> s_area6 = {
    { TerrainType::BIG_ISLAND,     200, 100 },
    { TerrainType::CARRIER_SHIP,   440, 240 },
    { TerrainType::CARRIER_SHIP,   680, 150 },
    { TerrainType::BIG_ISLAND,     920,  60 },
    { TerrainType::CARRIER_SHIP,  1160, 270 },
    { TerrainType::CARRIER_SHIP,  1400, 180 },
    { TerrainType::BIG_ISLAND,    1640, 120 },
    { TerrainType::CARRIER_SHIP,  1880, 220 },
    { TerrainType::CARRIER_SHIP,  2120, 300 },
    { TerrainType::CARRIER_SHIP,  2360,  80 },
};

// Area 7 — Okinawa/Tokyo: urban coastline, wall-to-wall carriers
static const std::vector<LevelObject> s_area7 = {
    { TerrainType::CARRIER_SHIP,   100, 200 },
    { TerrainType::CARRIER_SHIP,   280, 310 },
    { TerrainType::CARRIER_SHIP,   460, 130 },
    { TerrainType::BIG_ISLAND,     680,  80 },
    { TerrainType::CARRIER_SHIP,   880, 250 },
    { TerrainType::CARRIER_SHIP,  1080, 170 },
    { TerrainType::CARRIER_SHIP,  1280, 300 },
    { TerrainType::CARRIER_SHIP,  1480, 100 },
    { TerrainType::BIG_ISLAND,    1680,  60 },
    { TerrainType::CARRIER_SHIP,  1880, 220 },
    { TerrainType::CARRIER_SHIP,  2080, 280 },
    { TerrainType::CARRIER_SHIP,  2280, 140 },
};

const std::vector<LevelObject>& getLevelScript(int area) {
    switch (area) {
    case 1:  return s_area1;
    case 2:  return s_area2;
    case 3:  return s_area3;
    case 4:  return s_area4;
    case 5:  return s_area5;
    case 6:  return s_area6;
    case 7:  return s_area7;
    default: return s_area0;
    }
}
