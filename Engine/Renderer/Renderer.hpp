#pragma once

namespace Renderer
{
    void Init();

    void SetViewport(int x, int y, int width, int height);
    void SetClearColor(float r, float g, float b, float a);
    void Clear();
}