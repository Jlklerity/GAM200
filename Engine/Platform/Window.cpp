#include "Platform/Window.hpp"
#include "Application/Platform.hpp"

namespace
{
    SDL_Window* s_window = nullptr;
    SDL_GLContext s_context = nullptr;
    bool          s_shouldClose = false;
}

bool Window::Create(const char* title, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    s_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!s_window) return false;

    s_context = SDL_GL_CreateContext(s_window);
    if (!s_context) return false;

    // GL functions can only be loaded once a context exists.
#ifdef PLATFORM_NEEDS_GL_LOADER
    if (gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress) == 0) return false;
#endif

    SDL_GL_SetSwapInterval(1);
    return true;
}

void Window::Destroy()
{
    if (s_context) SDL_GL_DeleteContext(s_context);
    if (s_window)  SDL_DestroyWindow(s_window);
    s_context = nullptr;
    s_window = nullptr;
    SDL_Quit();
}

void Window::PollEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            s_shouldClose = true;
    }
}

bool Window::ShouldClose()
{
    return s_shouldClose;
}

void Window::SwapBuffers()
{
    SDL_GL_SwapWindow(s_window);
}

void Window::GetFramebufferSize(int* width, int* height)
{
    SDL_GL_GetDrawableSize(s_window, width, height);
}