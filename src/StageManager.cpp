#include "StageManager.h"
#include <algorithm>

// Indexed as [32 - stageNumber], so stage 32 is at [0], stage 1 is at [31].
static const StageDef s_stages[32] = {
    {32, "Midway",   false, 0},
    {31, "Midway",   false, 0},
    {30, "Midway",   false, 0},
    {29, "Midway",   false, 0},
    {28, "Marshall", false, 0},
    {27, "Marshall", false, 0},
    {26, "Marshall", true,  0},
    {25, "Marshall", false, 0},
    {24, "Attu",     false, 1},
    {23, "Attu",     false, 1},
    {22, "Attu",     false, 1},
    {21, "Attu",     false, 1},
    {20, "Rabaul",   false, 1},
    {19, "Rabaul",   false, 1},
    {18, "Rabaul",   true,  1},
    {17, "Rabaul",   false, 1},
    {16, "Leyte",    false, 2},
    {15, "Leyte",    false, 2},
    {14, "Leyte",    false, 2},
    {13, "Leyte",    false, 2},
    {12, "Saipan",   false, 2},
    {11, "Saipan",   false, 2},
    {10, "Saipan",   true,  2},
    { 9, "Saipan",   false, 2},
    { 8, "Iwojima",  false, 3},
    { 7, "Iwojima",  false, 3},
    { 6, "Iwojima",  false, 3},
    { 5, "Iwojima",  false, 3},
    { 4, "Okinawa",  false, 3},
    { 3, "Okinawa",  false, 3},
    { 2, "Okinawa",  true,  3},
    { 1, "Okinawa",  false, 3},
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
