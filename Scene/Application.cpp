#include "Application.hpp"


Application::Application(const char* title, int width, int height)
    : m_window{nullptr}, m_context{nullptr}, m_title{title}, m_width{width}, m_height{height},
      m_running{false}, m_VAO{0}, m_VBO{0}, m_EBO{0}, m_indexCount{0},
      m_shaderProgram{0}, m_texture{0}, m_locTex2d{-1}, m_locProjection{-1},
      m_locUseTexture{-1}, m_cameraPos{0.0f, 0.0f, 0.0f}, m_useTexture{true} {}

bool Application::Initialize() 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  24);

    m_window = SDL_CreateWindow(m_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                m_width, m_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_window) return false;

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context) return false;

#ifdef PLATFORM_NEEDS_GL_LOADER
    if (gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress) == 0) return false;
#endif

    SDL_GL_SetSwapInterval(1);
    SDL_GL_GetDrawableSize(m_window, &m_width, &m_height);

    VertexSpecification();
    CreateGraphicsPipeline();
    m_texture = AssetManager::setup_texobj(AssetManager::ImagePath("images.png"));

    return true;
}      

void Application::VertexSpecification() 
{
    const std::vector<GLfloat> vertexData{
        -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,   0.8f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,
    };
    const std::vector<GLuint> indexData{ 2,0,1, 3,2,1 };

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW);

    m_indexCount = static_cast<GLsizei>(indexData.size());
    GLsizei stride = 9 * sizeof(GLfloat);  
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat))); 
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(GLfloat)));
    glBindVertexArray(0);
}

GLuint Application::CompileShader(GLuint type, const std::string& source) 
{
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    return shader;
}

GLuint Application::CreateShaderProgram(const std::string& vSource, const std::string& fSource) 
{
    GLuint program = glCreateProgram();
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fSource);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void Application::CreateGraphicsPipeline() 
{
    std::string vSource = AssetManager::LoadShaderAsString(AssetManager::ShaderPath("vertexshader.glsl"));
    std::string fSource = AssetManager::LoadShaderAsString(AssetManager::ShaderPath("fragmentshader.glsl"));

    m_shaderProgram = CreateShaderProgram(vSource, fSource);

    m_locTex2d = glGetUniformLocation(m_shaderProgram, "uTex2d");
    if (m_locTex2d >= 0) 
    {
        glUseProgram(m_shaderProgram);
        glUniform1i(m_locTex2d, 0);
        glUseProgram(0);
    }

    m_locView = glGetUniformLocation(m_shaderProgram, "u_View");
    m_locProjection = glGetUniformLocation(m_shaderProgram, "u_Projection");
    m_locUseTexture = glGetUniformLocation(m_shaderProgram, "u_useTexture");
}

void Application::PreDraw() 
{
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Application::Draw() 
{
    glUseProgram(m_shaderProgram);

    float aspect = static_cast<float>(m_width) / static_cast<float>(m_height > 0 ? m_height : 1);

    glm::mat4 projection{};
    if (m_cam == Camera_mode::ortho_mode) 
    {
        projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
    }
    else if (m_cam == Camera_mode::persp_mode)
    {
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    }
    glm::mat4 view = glm::translate(glm::mat4(1.0f), -m_cameraPos);
    
    
    if (m_locProjection >= 0) glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(projection));
    if (m_locView >= 0) glUniformMatrix4fv(m_locView, 1, GL_FALSE, glm::value_ptr(view));
    if (m_locUseTexture >= 0) glUniform1i(m_locUseTexture, m_useTexture ? 1 : 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);      
    glUseProgram(0);
}

void Application::Frame() 
{
    static Uint64 previous = SDL_GetPerformanceCounter();
    const Uint64 now = SDL_GetPerformanceCounter();
    const float dt = static_cast<float>(static_cast<double>(now - previous) / static_cast<double>(SDL_GetPerformanceFrequency()));
    previous = now;

    InputManager::PollEvents(m_running, &m_width, &m_height, m_window);
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
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_texture) glDeleteTextures(1, &m_texture);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);

    if (m_context) SDL_GL_DeleteContext(m_context);
    if (m_window)  SDL_DestroyWindow(m_window);
    SDL_Quit();
}