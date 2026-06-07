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

    // Analog move direction [-1,1] (touch joystick or keyboard)
    float axisX() const { return m_axisX; }
    float axisY() const { return m_axisY; }

private:
    InputManager() = default;
    InputManager(const InputManager&) = delete;

    void handleKey(SDL_Keycode key, bool down);
    void handleFinger(const SDL_Event& e);

    std::unordered_map<Action, bool> m_held;
    std::unordered_map<Action, bool> m_pressed;
    std::unordered_map<Action, bool> m_released;

    // Touch state
    float m_touchStartX = 0, m_touchStartY = 0;
    bool  m_touching = false;
    float m_axisX = 0, m_axisY = 0;
};
