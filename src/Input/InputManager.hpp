#pragma once
#include "Application/pch.hpp"

enum class Key
{
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Up, Down, Left, Right,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Escape, Enter, Backspace,
    LeftControl, RightControl,
    LeftShift, RightShift,
    Count    
};

namespace InputManager {
    
    void PollEvents(bool& running, int* screenwidth, int* screenheight, SDL_Window* window);
    void Update(float dt, bool& running, glm::vec3& cameraPos, bool& u_useTexture);

    bool IsKeyDown(Key key);       
    bool IsKeyPressed(Key key);
    void EndFrame();
};