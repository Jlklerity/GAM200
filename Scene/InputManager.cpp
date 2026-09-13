#include "InputManager.hpp"
#include "Application.hpp"

void InputManager::PollEvents(bool& running, int* screenwidth, int* screenheight, SDL_Window* window)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:                       // window close button, Alt+F4
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
    // Pointer into SDL's internal array; valid for the program's lifetime,
    // refreshed by SDL_PumpEvents (which SDL_PollEvent calls for you).
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
    if (keys[SDL_SCANCODE_R]) cameraPos.z += speed * dt;
    if (keys[SDL_SCANCODE_F]) cameraPos.z -= speed * dt;

    static bool wasPressed_V = false;
    bool isPressed_V = keys[SDL_SCANCODE_V];

    if (isPressed_V && !wasPressed_V) 
    {
        // Explicitly toggle the strongly-typed enum
        if (Application::m_cam == Application::Camera_mode::ortho_mode) {
            Application::m_cam = Application::Camera_mode::persp_mode;
        } else {
            Application::m_cam = Application::Camera_mode::ortho_mode;
        }
    }
    wasPressed_V = isPressed_V;

    static bool wasPressed_P = false;
    bool isPressed_P = keys[SDL_SCANCODE_P];

    if (isPressed_P && !isPressed_P)
    {
        u_useTexture = !u_useTexture;
    }
    wasPressed_P = isPressed_P;

    cameraPos.x = std::clamp(cameraPos.x, -0.5f, 0.5f);
    cameraPos.y = std::clamp(cameraPos.y, -0.5f, 0.5f);
}