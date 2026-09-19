#pragma once
#include "Application/pch.hpp"

class InputManager {
public:
    static void PollEvents(bool& running, int* screenwidth, int* screenheight, SDL_Window* window);
    static void Update(float dt, bool& running, glm::vec3& cameraPos, bool& u_useTexture);
};