#include "StageManager.h"
#include <algorithm>

// Indexed as [32 - stageNumber], so stage 32 is at [0], stage 1 is at [31].
// Bosses every 4 stages (stages 29, 25, 21, 17, 13, 9, 5, 1) — first boss after 3 clears.
static const StageDef s_stages[32] = {
    {32, "Midway",   false, 0},
    {31, "Midway",   false, 0},
    {30, "Midway",   false, 0},
    {29, "Midway",   true,  0},  // Boss 1
    {28, "Marshall", false, 1},
    {27, "Marshall", false, 1},
    {26, "Marshall", false, 1},
    {25, "Marshall", true,  1},  // Boss 2
    {24, "Attu",     false, 2},
    {23, "Attu",     false, 2},
    {22, "Attu",     false, 2},
    {21, "Attu",     true,  2},  // Boss 3
    {20, "Rabaul",   false, 3},
    {19, "Rabaul",   false, 3},
    {18, "Rabaul",   false, 3},
    {17, "Rabaul",   true,  3},  // Boss 4
    {16, "Leyte",    false, 4},
    {15, "Leyte",    false, 4},
    {14, "Leyte",    false, 4},
    {13, "Leyte",    true,  4},  // Boss 5
    {12, "Saipan",   false, 5},
    {11, "Saipan",   false, 5},
    {10, "Saipan",   false, 5},
    { 9, "Saipan",   true,  5},  // Boss 6
    { 8, "Iwojima",  false, 6},
    { 7, "Iwojima",  false, 6},
    { 6, "Iwojima",  false, 6},
    { 5, "Iwojima",  true,  6},  // Boss 7
    { 4, "Okinawa",  false, 7},
    { 3, "Okinawa",  false, 7},
    { 2, "Okinawa",  false, 7},
    { 1, "Okinawa",  true,  7},  // Boss 8 (final)
};

StageManager& StageManager::get() {
    static StageManager inst;
    return inst;
}

void StageManager::reset() {
    m_current = 32;
}

void StageManager::resetToStage(int stage) {
    m_current = std::clamp(stage, 1, 32);
}

void StageManager::advance() {
    if (m_current > 1) --m_current;
}

const StageDef& StageManager::currentDef() const {
    return getDef(m_current);
}

const StageDef& StageManager::getDef(int stageNumber) const {
    int n = std::clamp(stageNumber, 1, 32);
    return s_stages[32 - n];
}
