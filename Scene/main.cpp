#include "pch.hpp"

// --- Window / GL context -------------------------------------------------
SDL_Window*   g_window  = nullptr;
SDL_GLContext g_context = nullptr;
bool          g_running = false;
GLuint        gTexObj   = 0;
GLint         gLocTex2d = -1;

int gScreenWidth  = 640;
int gScreenHeight = 480;
const char* title = "GAM200";

// --- Geometry ------------------------------------------------------------
GLuint  VAO = 0;
GLuint  VBO = 0;          // was a local; kept so CleanUp can delete it
GLuint  EBO = 0;
GLsizei gIndexCount = 0;  // set in VertexSpecification (segment 7, not yet applied)

// --- Shader --------------------------------------------------------------
GLuint gGraphicsPipelineShaderProgram = 0;
GLint  gLocOffsetX = -1;  // cached at link time (segment 6, not yet applied)
GLint  gLocOffsetY = -1;

// --- Game state ----------------------------------------------------------
float g_uOffsetX = 0.0f;
float g_uOffsetY = 0.0f;

void CleanUp();

void GetOpenGLVersionInfo()
{
    std::cout<<"Vendor: "          <<glGetString(GL_VENDOR)<<'\n';
    std::cout<<"Renderer: "        <<glGetString(GL_RENDERER)<<'\n';
    std::cout<<"Version: "         <<glGetString(GL_VERSION)<<'\n';
    std::cout<<"Shading Language: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<'\n';
}

void VertexSpecification()
{
    //cpu
    const std::vector<GLfloat> vertexData{

       -0.5f, -0.5f, 0.0f,   // Position
        1.0f,  0.0f, 0.0f,   // Color
        0.0f,  0.0f,         // TexCoord

       -0.5f,  0.5f, 0.0f,
        0.0f,  0.0f, 1.0f,
        0.0f,  1.0f,

        0.5f, -0.5f, 0.0f,
        0.8f,  1.0f, 0.0f,
        1.0f,  0.0f,

        0.5f,  0.5f, 0.0f,
        0.0f,  0.0f, 1.0f,
        1.0f,  1.0f,

    };

    const std::vector<GLuint> indexData{
        2,0,1,
        3,2,1
    };
 

    //gpu
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    GLsizeiptr vertexSize = vertexData.size() * sizeof(GLfloat);
    glBufferData(GL_ARRAY_BUFFER, vertexSize, vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    GLsizeiptr indexSize = indexData.size() * sizeof(GLuint);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexData.data(), GL_STATIC_DRAW);

    gIndexCount = static_cast<GLsizei>(indexData.size());

    GLsizei stride = 8 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));

    // Unbind last. The attribute enable bits and the element-buffer binding
    // are VAO state, already captured above -- the old glDisableVertexAttribArray
    // calls after this line acted on VAO 0, not on ours.
    glBindVertexArray(0);
}

GLuint setup_texobj(std::string const& tex_path)
{
    SDL_RWops* rw = SDL_RWFromFile(tex_path.c_str(), "rb");
    if (rw == nullptr)
    {
        LOGE("setup_texobj: failed to open texture file '%s': %s",
             tex_path.c_str(), SDL_GetError());
        return 0;
    }

    const Sint64 size = SDL_RWsize(rw);
    if (size <= 0)
    {
        LOGE("setup_texobj: '%s' is empty or unsized: %s",
             tex_path.c_str(), SDL_GetError());
        SDL_RWclose(rw);
        return 0;
    }

    std::vector<stbi_uc> encoded(static_cast<size_t>(size));
    const size_t got = SDL_RWread(rw, encoded.data(), 1, encoded.size());
    SDL_RWclose(rw);

    if (got != encoded.size())
    {
        LOGE("setup_texobj: short read on '%s' (%lld of %lld bytes)",
             tex_path.c_str(), (long long)got, (long long)encoded.size());
        return 0;
    }

    // GL samples with the origin at the bottom-left; PNG and JPEG both store
    // the top row first. The .tex pipeline baked this flip in at conversion
    // time -- with encoded formats it has to happen at load.
    stbi_set_flip_vertically_on_load(1);

    int width = 0, height = 0, channels_in_file = 0;
    // The trailing 4 forces RGBA whatever the file holds, so the upload format
    // is fixed and a 3-channel image with an odd width cannot trip the default
    // GL_UNPACK_ALIGNMENT of 4 and shear diagonally.
    stbi_uc* ptr_texels = stbi_load_from_memory(
        encoded.data(), (static_cast<int>(encoded.size())),
        &width, &height, &channels_in_file, 4);

    if (ptr_texels == nullptr)
    {
        LOGE("setup_texobj: decode failed for '%s': %s",
             tex_path.c_str(), stbi_failure_reason());
        return 0;
    }

    GLuint texobj_hdl{};
    glGenTextures(1, &texobj_hdl);
    glBindTexture(GL_TEXTURE_2D, texobj_hdl);

    // allocate GPU storage for texture image data loaded from file
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    // copy image data from client memory to GPU texture buffer memory
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE, ptr_texels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);
    // client memory not required since image is buffered in GPU memory
    stbi_image_free(ptr_texels);

    LOGI("setup_texobj: '%s' %dx%d (%d channels in file)",
         tex_path.c_str(), width, height, channels_in_file);
    return texobj_hdl;
}

// ---------------------------------------------------------------------------
// Asset paths
//
// SDL_RWFromFile resolves a relative path differently on each target, and that
// difference is what makes one code path work everywhere:
//
//   Desktop     relative to the process CWD. SDL_GetBasePath() returns the
//               directory holding the executable -- the same directory CMake's
//               POST_BUILD step copies shaders/ into -- so prepending it makes
//               the lookup independent of where the exe was launched from.
//   Emscripten  SDL_GetBasePath() returns "/", and build_web's
//               --preload-file shaders mounts the folder at /shaders, so the
//               same concatenation lands on /shaders/x.glsl.
//
// This replaces the old "../../shaders/..." literal, which only worked because
// Emscripten clamps paths that climb above the root.
// ---------------------------------------------------------------------------
std::string AssetPath(const std::string& folder, const std::string& name)
{
    static const std::string base = []() -> std::string
    {
        char* p = SDL_GetBasePath();
        if (p == nullptr)
        {
            return {};              // fall back to the CWD
        }
        std::string s{p};
        SDL_free(p);                // SDL_GetBasePath allocates; we own it
        return s;
    }();
    return base + folder + "/" + name;
}

std::string ShaderPath(const std::string& name) { return AssetPath("shaders", name); }
std::string ImagePath (const std::string& name) { return AssetPath("images",  name); }

// Returns the file's contents, or an empty string on failure. Unlike the old
// ifstream version this reports WHY it failed: an empty return used to reach
// glShaderSource as empty source, so the visible error was a shader compile
// error rather than "file not found".
std::string LoadShaderAsString(const std::string& path)
{
    SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "rb");
    if (rw == nullptr)
    {
        LOGE("LoadShaderAsString: cannot open '%s': %s", path.c_str(), SDL_GetError());
        return {};
    }

    const Sint64 size = SDL_RWsize(rw);
    if (size <= 0)
    {
        LOGE("LoadShaderAsString: '%s' is empty or unsized: %s", path.c_str(), SDL_GetError());
        SDL_RWclose(rw);
        return {};
    }

    std::string result(static_cast<size_t>(size), '\0');
    const size_t got = SDL_RWread(rw, result.data(), 1, static_cast<size_t>(size));
    SDL_RWclose(rw);

    if (got != static_cast<size_t>(size))
    {
        LOGE("LoadShaderAsString: short read on '%s' (%lld of %lld bytes)",
             path.c_str(), (long long)got, (long long)size);
        return {};
    }

    return result;
}

GLuint CompileShader(GLuint type, const std::string& source)
{
    GLuint shaderObject = 0;
    if(type == GL_VERTEX_SHADER)
    {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    }
    else if(type == GL_FRAGMENT_SHADER)
    {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }
    const char* src = source.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);
    
    GLint isCompiled = 0;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shaderObject, maxLength, &maxLength, &errorLog[0]);
        std::cout << "Shader compilation failed:\n" << &errorLog[0] << '\n';
        glDeleteShader(shaderObject);
    }

    return shaderObject;
}

GLuint CreateShaderProgram(const std::string& vertex_shadersource, const std::string& fragment_shadersource)
{
    GLuint programObject    = glCreateProgram();
    GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertex_shadersource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragment_shadersource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    // Check for linking errors
    GLint isLinked = 0;
    glGetProgramiv(programObject, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(programObject, maxLength, &maxLength, &infoLog[0]);
        std::cout << "Shader linking failed:\n" << &infoLog[0] << '\n';
        
        glDeleteProgram(programObject);
        return 0;
    }
    
    glDetachShader(programObject, myVertexShader);
    glDetachShader(programObject, myFragmentShader);
    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);
    
    return programObject;
}

void CreateGraphicsPipeline()
{
    const std::string vertexShaderSource   = LoadShaderAsString(ShaderPath("vertexshader.glsl"));
    const std::string fragmentShaderSource = LoadShaderAsString(ShaderPath("fragmentshader.glsl"));

    // Bail before the GL calls. Handing empty source to glShaderSource reports
    // a compile error and hides the real cause, which LoadShaderAsString has
    // already logged.
    if (vertexShaderSource.empty() || fragmentShaderSource.empty())
    {
        LOGE("CreateGraphicsPipeline: shader source missing, aborting");
        CleanUp();
        exit(1);
    }

    gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (gGraphicsPipelineShaderProgram == 0)
    {
        LOGE("CreateGraphicsPipeline: shader program creation failed");
        CleanUp();
        exit(1);
    }

    gLocTex2d = glGetUniformLocation(gGraphicsPipelineShaderProgram, "uTex2d");
    if (gLocTex2d < 0) std::cout << "warning: uTex2d not found\n";

    if (gLocTex2d >= 0)
    {
        glUseProgram(gGraphicsPipelineShaderProgram);
        glUniform1i(gLocTex2d, 0);
        glUseProgram(0);
    }

    // Locations are fixed once the program is linked. Query them here, never
    // per frame.
    gLocOffsetX = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_offsetX");
    gLocOffsetY = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_offsetY");

    if (gLocOffsetX < 0) std::cout << "warning: u_offsetX not found\n";
    if (gLocOffsetY < 0) std::cout << "warning: u_offsetY not found\n";
}

void InitializeProgram() //Start SDL2 and create the game window (OpenGL ES 3.0 to work with Android)
{
    // If you later ship ANGLE's libEGL.dll + libGLESv2.dll next to the .exe,
    // uncomment this to force a true ES driver. Leave it off for now: without
    // those DLLs present it makes context creation fail outright.
    // SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

    if (SDL_Init(SDL_INIT_VIDEO) < 0)   // SDL2 returns 0 on SUCCESS
    {
        std::cout << "SDL_Init failed: " << SDL_GetError() << '\n';
        exit(1);
    }

    // All GL attributes must be set BEFORE SDL_CreateWindow.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  24);   // SDL2 defaults to 16

    g_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        gScreenWidth, gScreenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (g_window == nullptr)
    {
        std::cout << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        SDL_Quit();                       // no window, no context to destroy
        exit(1);
    }

    g_context = SDL_GL_CreateContext(g_window);
    if (g_context == nullptr)
    {
        std::cout << "SDL_GL_CreateContext failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        exit(1);
    }

#ifdef PLATFORM_NEEDS_GL_LOADER
    if (gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress) == 0)
    {
        std::cout << "Failed to load OpenGL ES 3.0 function pointers\n";
        CleanUp();
        exit(1);
    }
#endif

    SDL_GL_SetSwapInterval(1);   // ignored on web; the browser drives the loop

    // Window size is in points; the framebuffer is in pixels. On a HiDPI
    // display these differ, and glViewport wants pixels.
    SDL_GL_GetDrawableSize(g_window, &gScreenWidth, &gScreenHeight);

    #if GET_VERSION  // add GET_VERSION flag when running bat script to receive OpenGL info
    GetOpenGLVersionInfo();
    #endif
}

void PollEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:                       // window close button, Alt+F4
            g_running = false;
            break;

        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                SDL_GL_GetDrawableSize(g_window, &gScreenWidth, &gScreenHeight);
            }
            break;

        default:
            break;
        }
    }
}

void Input(float dt)
{
    // Pointer into SDL's internal array; valid for the program's lifetime,
    // refreshed by SDL_PumpEvents (which SDL_PollEvent calls for you).
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_ESCAPE])
    {
        g_running = false;
        std::cout << "program ended!\n";
    }

    const float speed = 0.6f;   // NDC units per second (~0.01 per frame at 60Hz)

    if (keys[SDL_SCANCODE_W]) g_uOffsetY += speed * dt;
    if (keys[SDL_SCANCODE_S]) g_uOffsetY -= speed * dt;
    if (keys[SDL_SCANCODE_A]) g_uOffsetX -= speed * dt;
    if (keys[SDL_SCANCODE_D]) g_uOffsetX += speed * dt;

    g_uOffsetX = std::clamp(g_uOffsetX, -0.5f, 0.5f);
    g_uOffsetY = std::clamp(g_uOffsetY, -0.5f, 0.5f);
}

void PreDraw()
{
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Draw()
{
    glUseProgram(gGraphicsPipelineShaderProgram);

    if (gLocOffsetX >= 0) glUniform1f(gLocOffsetX, g_uOffsetX);
    if (gLocOffsetY >= 0) glUniform1f(gLocOffsetY, g_uOffsetY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexObj);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, gIndexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);      
    glUseProgram(0);
}

void Frame()
{
    static Uint64 previous = SDL_GetPerformanceCounter();

    const Uint64 now = SDL_GetPerformanceCounter();

    // Both operands are Uint64. Divide in integer arithmetic and dt is
    // always 0 -- cast to double BEFORE dividing.
    const float dt = static_cast<float>(
        static_cast<double>(now - previous) /
        static_cast<double>(SDL_GetPerformanceFrequency()));

    previous = now;

    PollEvents();   // must precede Input(): it pumps the keyboard state
    Input(dt);
    PreDraw();
    Draw();
    SDL_GL_SwapWindow(g_window);   // Update the screen
}

void MainLoop()
{
    g_running = true;

#ifdef PLATFORM_EMSCRIPTEN
    // Browser owns the loop. fps=0 means "use requestAnimationFrame".
    // simulate_infinite_loop=1 means this call never returns.
    emscripten_set_main_loop(Frame, 0, 1);
#else
    while (g_running)
    {
        Frame();
    }
#endif
}

void CleanUp()
{
    if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
    if (VBO) { glDeleteBuffers(1, &VBO);      VBO = 0; }
    if (EBO) { glDeleteBuffers(1, &EBO);      EBO = 0; }
    if (gTexObj) { glDeleteTextures(1, &gTexObj); gTexObj = 0; }

    if (gGraphicsPipelineShaderProgram)
    {
        glDeleteProgram(gGraphicsPipelineShaderProgram);
        gGraphicsPipelineShaderProgram = 0;
    }

    if (g_context) { SDL_GL_DeleteContext(g_context); g_context = nullptr; }
    if (g_window)  { SDL_DestroyWindow(g_window);     g_window  = nullptr; }

    SDL_Quit();
}

//main
// SDL2 #defines main to SDL_main; SDL2main supplies the real entry point.
// This signature must match EXACTLY or it will not link on Windows.
int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    InitializeProgram();
    VertexSpecification();
    CreateGraphicsPipeline();
    gTexObj = setup_texobj(ImagePath("images.png"));
    MainLoop();

#ifndef PLATFORM_EMSCRIPTEN
    CleanUp();   // unreachable on web: emscripten_set_main_loop never returns
#endif
    return 0;
}