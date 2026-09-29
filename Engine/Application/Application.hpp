#pragma once

class Scene;

class Application {
public:

    explicit Application(const char* title, int width, int height);
    ~Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    bool Initialize(Scene& scene);
    void Run();
    void CleanUp();

    int   GetScreenWidth() const;
    int   GetScreenHeight() const;

private:
    void Frame();
    void Update();
    void BeginDraw() const;
    void EndDraw();
    bool IsRunning() const;

    static Application* s_instance;
    static bool         s_initialized;

    Scene* m_scene;
    const char* m_title;
    int           m_screenWidth;
    int           m_screenHeight;
    float         m_deltaTime;
    double        m_lastFrameTime;
};