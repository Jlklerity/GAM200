#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "pch.hpp"
#include "AssetManager.hpp"
#include "InputManager.hpp"

class Application {
public:
    // Camera Mode 
    enum class Camera_mode : bool
    {
        ortho_mode = 0,
        persp_mode
    };
    inline static Camera_mode m_cam = Camera_mode::ortho_mode;

    Application(const char* title, int width, int height);
    bool Initialize();
    void MainLoop();
    void CleanUp();

private:
    void Frame();
    static void EmscriptenLoop(void* arg);
    
    void VertexSpecification();
    GLuint CompileShader(GLuint type, const std::string& source);
    GLuint CreateShaderProgram(const std::string& vertex_shadersource, const std::string& fragment_shadersource);
    void CreateGraphicsPipeline();

    void PreDraw();
    void Draw();

    // Window / Context
    SDL_Window*   m_window;
    SDL_GLContext m_context;
    const char*   m_title;
    int           m_width;
    int           m_height;
    bool          m_running;

    // Geometry
    GLuint  m_VAO;
    GLuint  m_VBO;
    GLuint  m_EBO;
    GLsizei m_indexCount;

    // Shader / Textures
    GLuint m_shaderProgram;
    GLuint m_texture;
    GLint  m_locTex2d;
    GLint m_locView;
    GLint m_locProjection;
    GLint  m_locUseTexture;

    // Game State
    glm::vec3 m_cameraPos;
    bool  m_useTexture;

};