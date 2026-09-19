#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Application/pch.hpp"
#include "Renderer/AssetManager.hpp"
#include "Input/InputManager.hpp"
#include "Scene/GameScene.hpp"


class Application {
public:

    Application(const char* title, int width, int height);
    bool Initialize();
    void MainLoop();
    void CleanUp();

    int           m_screenWidth;
    int           m_screenHeight;
private:
    void Frame();
    static void EmscriptenLoop(void* arg);
    
    void PreDraw();
    void Draw();

    std::unique_ptr<GameScene> m_scene;

    glm::vec3   m_cameraPos;
    bool        m_useTexture;

    // Window / Context
    SDL_Window*   m_window;
    SDL_GLContext m_context;
    const char*   m_title;
    bool          m_running;
};