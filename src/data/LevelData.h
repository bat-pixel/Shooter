#pragma once
#include <vector>

enum class TerrainType { SMALL_ISLAND, BIG_ISLAND, CARRIER_SHIP, BOAT };

struct LevelObject {
    TerrainType type;
    float worldY;   // world distance at which this object spawns
    float x;        // screen X position
};

// Returns the level script for the given area index (0-3).
// Stages within the same area share one script; it loops when exhausted.
const std::vector<LevelObject>& getLevelScript(int area);
