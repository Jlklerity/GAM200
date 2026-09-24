// InputManager.cpp
#include "Input/InputManager.hpp"
#include <array>

void InputManager::PollEvents(bool& running, int* screenwidth, int* screenheight, SDL_Window* window)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                SDL_GL_GetDrawableSize(window, screenwidth, screenheight);
            }
            break;

        default:
            break;
        }
    }
}

void InputManager::Update(float dt, bool& running, glm::vec3& cameraPos, bool& u_useTexture)
{
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_ESCAPE])
    {
        running = false;
        std::cout << "program ended!\n";
    }

    const float speed = 2.0f;

    if (keys[SDL_SCANCODE_W]) cameraPos.y += speed * dt;
    if (keys[SDL_SCANCODE_S]) cameraPos.y -= speed * dt;
    if (keys[SDL_SCANCODE_A]) cameraPos.x -= speed * dt;
    if (keys[SDL_SCANCODE_D]) cameraPos.x += speed * dt;
    if (keys[SDL_SCANCODE_R]) cameraPos.z -= speed * dt;
    if (keys[SDL_SCANCODE_F]) cameraPos.z += speed * dt;


    static bool wasPressed_P = false;
    bool isPressed_P = keys[SDL_SCANCODE_P];

    if (isPressed_P && !wasPressed_P)
    {
        u_useTexture = !u_useTexture;
    }
    wasPressed_P = isPressed_P;

    cameraPos.x = std::clamp(cameraPos.x, -0.5f, 0.5f);
    cameraPos.y = std::clamp(cameraPos.y, -0.5f, 0.5f);
}

namespace
{
    SDL_Scancode ToScancode(Key key)
    {
        switch (key)
        {
        case Key::A: return SDL_SCANCODE_A;
        case Key::B: return SDL_SCANCODE_B;
        case Key::C: return SDL_SCANCODE_C;
        case Key::D: return SDL_SCANCODE_D;
        case Key::E: return SDL_SCANCODE_E;
        case Key::F: return SDL_SCANCODE_F;
        case Key::G: return SDL_SCANCODE_G;
        case Key::H: return SDL_SCANCODE_H;
        case Key::I: return SDL_SCANCODE_I;
        case Key::J: return SDL_SCANCODE_J;
        case Key::K: return SDL_SCANCODE_K;
        case Key::L: return SDL_SCANCODE_L;
        case Key::M: return SDL_SCANCODE_M;
        case Key::N: return SDL_SCANCODE_N;
        case Key::O: return SDL_SCANCODE_O;
        case Key::P: return SDL_SCANCODE_P;
        case Key::Q: return SDL_SCANCODE_Q;
        case Key::R: return SDL_SCANCODE_R;
        case Key::S: return SDL_SCANCODE_S;
        case Key::T: return SDL_SCANCODE_T;
        case Key::U: return SDL_SCANCODE_U;
        case Key::V: return SDL_SCANCODE_V;
        case Key::W: return SDL_SCANCODE_W;
        case Key::X: return SDL_SCANCODE_X;
        case Key::Y: return SDL_SCANCODE_Y;
        case Key::Z:     return SDL_SCANCODE_Z;
        case Key::Up:    return SDL_SCANCODE_UP;
        case Key::Down:  return SDL_SCANCODE_DOWN;
        case Key::Left:  return SDL_SCANCODE_LEFT;
        case Key::Right: return SDL_SCANCODE_RIGHT;
        case Key::Num0: return SDL_SCANCODE_0;
        case Key::Num1: return SDL_SCANCODE_1;
        case Key::Num2: return SDL_SCANCODE_2;
        case Key::Num3: return SDL_SCANCODE_3;
        case Key::Num4: return SDL_SCANCODE_4;
        case Key::Num5: return SDL_SCANCODE_5;
        case Key::Num6: return SDL_SCANCODE_6;
        case Key::Num7: return SDL_SCANCODE_7;
        case Key::Num8: return SDL_SCANCODE_8;
        case Key::Num9: return SDL_SCANCODE_9;
        case Key::Escape:       return SDL_SCANCODE_ESCAPE;
        case Key::Enter:        return SDL_SCANCODE_RETURN;
        case Key::Backspace:    return SDL_SCANCODE_BACKSPACE;
        case Key::LeftControl:  return SDL_SCANCODE_LCTRL;
        case Key::RightControl: return SDL_SCANCODE_RCTRL;
        case Key::LeftShift:    return SDL_SCANCODE_LSHIFT;
        case Key::RightShift:   return SDL_SCANCODE_RSHIFT;
        default: return SDL_SCANCODE_UNKNOWN;   // Key::Count (or any future gap) lands here
        }
    }

    constexpr std::size_t kKeyCount = static_cast<std::size_t>(Key::Count);
    std::array<bool, kKeyCount> s_previousState{};
}

bool InputManager::IsKeyDown(Key key)
{
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    return keys[ToScancode(key)] != 0;
}

bool InputManager::IsKeyPressed(Key key)
{
    const bool isDown = IsKeyDown(key);
    const bool wasDown = s_previousState[static_cast<std::size_t>(key)];
    return isDown && !wasDown;
}

void InputManager::EndFrame()
{
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    for (std::size_t i = 0; i < kKeyCount; ++i)
        s_previousState[i] = keys[ToScancode(static_cast<Key>(i))] != 0;
}
