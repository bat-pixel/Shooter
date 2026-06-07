#include "StageManager.h"
#include <algorithm>

// Indexed as [32 - stageNumber], so stage 32 is at [0], stage 1 is at [31].
// Bosses every 4 stages (stages 29, 25, 21, 17, 13, 9, 5, 1) — first boss after 3 clears.
static const StageDef s_stages[32] = {
    {32, "Midway",   false, 0},
    {31, "Midway",   false, 0},
    {30, "Midway",   false, 0},
    {29, "Midway",   true,  0},  // Boss 1
    {28, "Marshall", false, 0},
    {27, "Marshall", false, 0},
    {26, "Marshall", false, 0},
    {25, "Marshall", true,  0},  // Boss 2
    {24, "Attu",     false, 1},
    {23, "Attu",     false, 1},
    {22, "Attu",     false, 1},
    {21, "Attu",     true,  1},  // Boss 3
    {20, "Rabaul",   false, 1},
    {19, "Rabaul",   false, 1},
    {18, "Rabaul",   false, 1},
    {17, "Rabaul",   true,  1},  // Boss 4
    {16, "Leyte",    false, 2},
    {15, "Leyte",    false, 2},
    {14, "Leyte",    false, 2},
    {13, "Leyte",    true,  2},  // Boss 5
    {12, "Saipan",   false, 2},
    {11, "Saipan",   false, 2},
    {10, "Saipan",   false, 2},
    { 9, "Saipan",   true,  2},  // Boss 6
    { 8, "Iwojima",  false, 3},
    { 7, "Iwojima",  false, 3},
    { 6, "Iwojima",  false, 3},
    { 5, "Iwojima",  true,  3},  // Boss 7
    { 4, "Okinawa",  false, 3},
    { 3, "Okinawa",  false, 3},
    { 2, "Okinawa",  false, 3},
    { 1, "Okinawa",  true,  3},  // Boss 8 (final)
};

StageManager& StageManager::get() {
    static StageManager inst;
    return inst;
}

void StageManager::reset() {
    m_current = 32;
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
