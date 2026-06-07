#include "InputManager.h"
#include "Constants.h"

InputManager& InputManager::get() {
    static InputManager instance;
    return instance;
}

void InputManager::endFrame() {
    m_pressedPowerUpSlot = 0;
    m_pressed.clear();
    m_released.clear();
    // Rebuild axis from held state (keyboard)
    if (!m_touching) {
        m_axisX = (m_held[Action::MOVE_RIGHT] ? 1.f : 0.f)
                - (m_held[Action::MOVE_LEFT]  ? 1.f : 0.f);
        m_axisY = (m_held[Action::MOVE_DOWN]  ? 1.f : 0.f)
                - (m_held[Action::MOVE_UP]    ? 1.f : 0.f);
    }
}

void InputManager::update(const SDL_Event& e) {
    switch (e.type) {
    case SDL_EVENT_KEY_DOWN:
        if (!e.key.repeat) handleKey(e.key.key, true);
        break;
    case SDL_EVENT_KEY_UP:
        handleKey(e.key.key, false);
        break;
    case SDL_EVENT_FINGER_DOWN:
    case SDL_EVENT_FINGER_MOTION:
    case SDL_EVENT_FINGER_UP:
        handleFinger(e);
        break;
    default:
        break;
    }
}

void InputManager::handleKey(SDL_Keycode key, bool down) {
    Action a;
    bool mapped = true;
    switch (key) {
    case SDLK_LEFT:  case SDLK_A: a = Action::MOVE_LEFT;  break;
    case SDLK_RIGHT: case SDLK_D: a = Action::MOVE_RIGHT; break;
    case SDLK_UP:    case SDLK_W: a = Action::MOVE_UP;    break;
    case SDLK_DOWN:  case SDLK_S: a = Action::MOVE_DOWN;  break;
    case SDLK_SPACE:              a = Action::FIRE;        break;
    case SDLK_X:                  a = Action::LOOP;        break;
    case SDLK_ESCAPE: case SDLK_P:a = Action::PAUSE;      break;
    case SDLK_RETURN:             a = Action::CONFIRM;     break;
    case SDLK_BACKSPACE:          a = Action::BACK;        break;
    case SDLK_T:                  a = Action::TRAINING;    break;
    case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
    case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8:
        if (down) m_pressedPowerUpSlot = (int)(key - SDLK_0);
        return;
    default: mapped = false; break;
    }
    if (!mapped) return;

    if (down) {
        if (!m_held[a]) m_pressed[a] = true;
        m_held[a] = true;
    } else {
        m_held[a]     = false;
        m_released[a] = true;
    }
}

void InputManager::handleFinger(const SDL_Event& e) {
    if (e.type == SDL_EVENT_FINGER_DOWN) {
        m_touching    = true;
        m_touchStartX = e.tfinger.x * LOGICAL_W;
        m_touchStartY = e.tfinger.y * LOGICAL_H;
        m_pressed[Action::FIRE] = true;
        m_held[Action::FIRE]    = true;
    } else if (e.type == SDL_EVENT_FINGER_MOTION) {
        float curX = e.tfinger.x * LOGICAL_W;
        float curY = e.tfinger.y * LOGICAL_H;
        float dx = curX - m_touchStartX;
        float dy = curY - m_touchStartY;
        float deadzone = 10.f;
        m_axisX = (SDL_fabsf(dx) > deadzone) ? SDL_clamp(dx / 80.f, -1.f, 1.f) : 0.f;
        m_axisY = (SDL_fabsf(dy) > deadzone) ? SDL_clamp(dy / 80.f, -1.f, 1.f) : 0.f;
    } else if (e.type == SDL_EVENT_FINGER_UP) {
        m_touching           = false;
        m_axisX = m_axisY    = 0;
        m_held[Action::FIRE] = false;
        m_released[Action::FIRE] = true;
    }
}

bool InputManager::isHeld(Action a) const {
    auto it = m_held.find(a);
    return it != m_held.end() && it->second;
}
bool InputManager::isPressed(Action a) const {
    auto it = m_pressed.find(a);
    return it != m_pressed.end() && it->second;
}
bool InputManager::isReleased(Action a) const {
    auto it = m_released.find(a);
    return it != m_released.end() && it->second;
}
