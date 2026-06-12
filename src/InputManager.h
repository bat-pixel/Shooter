#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>

enum class Action {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    FIRE,
    LOOP,       // loop-the-loop / special
    PAUSE,
    CONFIRM,
    BACK,
    TRAINING,
};

class InputManager {
public:
    static InputManager& get();

    // Call once per frame before game update
    void update(const SDL_Event& event);
    void endFrame();

    bool isHeld(Action a) const;
    bool isPressed(Action a) const;   // true only on the frame it was pressed
    bool isReleased(Action a) const;

    // Analog move direction [-1,1] from keyboard
    float axisX() const { return m_axisX; }
    float axisY() const { return m_axisY; }

    // Direct touch position in logical screen coordinates (updated every frame)
    bool  isTouching() const { return m_touching; }
    float touchX()     const { return m_touchX; }
    float touchY()     const { return m_touchY; }

    int pressedPowerUpSlot() const { return m_pressedPowerUpSlot; }

private:
    InputManager() = default;
    InputManager(const InputManager&) = delete;

    void handleKey(SDL_Keycode key, bool down);
    void handleFinger(const SDL_Event& e);

    std::unordered_map<Action, bool> m_held;
    std::unordered_map<Action, bool> m_pressed;
    std::unordered_map<Action, bool> m_released;

    int   m_pressedPowerUpSlot = 0;

    bool  m_touching   = false;
    float m_touchX     = 0, m_touchY     = 0; // absolute logical coords
    float m_touchStartX= 0, m_touchStartY= 0; // kept for axis fallback
    float m_axisX      = 0, m_axisY      = 0;
};
