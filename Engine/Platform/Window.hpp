#pragma once

namespace Window
{
    bool Create(const char* title, int width, int height);
    void Destroy();

    void PollEvents();
    bool ShouldClose();

    void SwapBuffers();
    void GetFramebufferSize(int* width, int* height);
}