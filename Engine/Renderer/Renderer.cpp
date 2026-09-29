#include "Renderer/Renderer.hpp"
#include "Application/Platform.hpp"

void Renderer::Init()
{
    glEnable(GL_DEPTH_TEST);
}

void Renderer::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void Renderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}