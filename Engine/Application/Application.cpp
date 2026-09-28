#include "Application.hpp"
#include "Input/InputManager.hpp"

Application::Application(const char* title, int width, int height)
    : m_window{nullptr}, m_context{nullptr}, m_title{title}, m_screenWidth{width}, m_screenHeight{height},
      m_running{false}, m_cameraPos{0.0f, 0.0f, 0.0f}, m_useTexture{true} {}

bool Application::Initialize() 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  24);

    m_window = SDL_CreateWindow(m_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                m_screenWidth, m_screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_window) return false;

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context) return false;

#ifdef PLATFORM_NEEDS_GL_LOADER
    if (gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress) == 0) return false;
#endif
    glEnable(GL_DEPTH_TEST);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_GetDrawableSize(m_window, &m_screenWidth, &m_screenHeight);

    m_scene = std::make_unique<GameScene>();
    m_scene->InitModel();
    m_scene->Resize(m_screenWidth, m_screenHeight);

    return true;
}      

void Application::PreDraw() 
{
    glViewport(0, 0, m_screenWidth, m_screenHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Application::Draw() 
{
    m_scene->Render(m_cameraPos, m_useTexture);
}

void Application::Frame() 
{
    static Uint64 previous = SDL_GetPerformanceCounter();
    const Uint64 now = SDL_GetPerformanceCounter();
    const float dt = static_cast<float>(static_cast<double>(now - previous) / static_cast<double>(SDL_GetPerformanceFrequency()));
    previous = now;

    InputManager::PollEvents(m_running, &m_screenWidth, &m_screenHeight, m_window);
    InputManager::Update(dt, m_running, m_cameraPos, m_useTexture);
    PreDraw();
    Draw();
    SDL_GL_SwapWindow(m_window);
}

void Application::EmscriptenLoop(void* arg) 
{
    static_cast<Application*>(arg)->Frame();
}

void Application::MainLoop() 
{
    m_running = true;
#ifdef PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop_arg(EmscriptenLoop, this, 0, 1);
#else
    while (m_running) { Frame(); }
#endif
}

void Application::CleanUp() 
{
    SDL_Quit();
}