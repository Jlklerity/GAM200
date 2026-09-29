#include "Application.hpp"
#include "Application/Platform.hpp"
#include "Application/Scene.hpp"
#include "Platform/Time.hpp"
#include "Platform/Window.hpp"
#include "Renderer/Renderer.hpp"
#include <cassert>
#include <cstdlib>

Application* Application::s_instance = nullptr;
bool         Application::s_initialized = false;


Application::Application(const char* title, int width, int height)
    : m_scene{ nullptr }, m_title{ title }, m_screenWidth{ width }, m_screenHeight{ height },
    m_deltaTime{ 0.0f }, m_lastFrameTime{ 0.0 }
{
    if (s_instance != nullptr)
    {
        LOGE("Application: only one Application can exist");
        std::fflush(stdout);   
        std::abort();
    }
    s_instance = this;
}

Application::~Application()
{
    s_instance = nullptr;
}

bool Application::Initialize(Scene& scene)
{
    if (s_initialized)
    {
        LOGE("Application::Initialize: the engine was already initialized");
        return false;
    }
    s_initialized = true;
    
    m_scene = &scene;

    if (!Window::Create(m_title, m_screenWidth, m_screenHeight)) return false;
    Window::GetFramebufferSize(&m_screenWidth, &m_screenHeight);

    Renderer::Init();

    m_scene->InitModel();
    m_scene->Resize(m_screenWidth, m_screenHeight);

    m_lastFrameTime = Time::GetTime();
    return true;
}

void Application::Run()
{
#ifdef PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop_arg([](void* arg)
        {
            Application* app = static_cast<Application*>(arg);
            if (!app->IsRunning())
            {
                app->CleanUp();
                emscripten_cancel_main_loop();
                return;
            }
            app->Frame();
        }, this, 0, 1);   
#else
    while (IsRunning())
    {
        Frame();
    }
#endif
}

void Application::CleanUp()
{
    if (m_scene == nullptr) return;
    
    m_scene->CleanUp();                 
    m_scene = nullptr;
    Window::Destroy();
}

void Application::Frame()
{
    Update();
    m_scene->Update(m_deltaTime);

    BeginDraw();
    m_scene->Draw();
    EndDraw();
}

void Application::Update()
{
    const double now = Time::GetTime();
    m_deltaTime = static_cast<float>(now - m_lastFrameTime);
    m_lastFrameTime = now;

    Window::PollEvents();

    int width = 0, height = 0;
    Window::GetFramebufferSize(&width, &height);
    if (width != m_screenWidth || height != m_screenHeight)
    {
        m_screenWidth = width;
        m_screenHeight = height;
        m_scene->Resize(m_screenWidth, m_screenHeight);
    }
}

void Application::BeginDraw() const
{
    Renderer::SetViewport(0, 0, m_screenWidth, m_screenHeight);
    Renderer::SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    Renderer::Clear();
}

void Application::EndDraw()
{
    Window::SwapBuffers();
}

bool Application::IsRunning() const
{
    return m_scene->IsRunning() && !Window::ShouldClose();
}

int Application::GetScreenWidth() const
{
    return m_screenWidth;
}

int Application::GetScreenHeight() const
{
    return m_screenHeight;
}