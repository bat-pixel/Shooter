#pragma once

struct StageDef {
    int         stageNumber;
    const char* campaign;
    bool        hasBoss;
    int         bgIndex;  // 0=darkPurple, 1=blue, 2=purple, 3=black
};

class StageManager {
public:
    static StageManager& get();

    void reset();
    void resetToStage(int stage);
    void advance();

    int             currentStage() const { return m_current; }
    const StageDef& currentDef()   const;
    const StageDef& getDef(int stageNumber) const;

    int  highScore()                const { return m_highScore; }
    void updateHighScore(int score)       { if (score > m_highScore) m_highScore = score; }

private:
    StageManager() = default;
    int m_current   = 32;
    int m_highScore = 0;
};
