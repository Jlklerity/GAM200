#pragma once
#include <vector>
#include "Scene/Entity.hpp"
#include "Application/Platform.hpp"
#include "Renderer/Mesh.hpp"
#include "Input/InputManager.hpp"
#include "Renderer/ShaderHelper.hpp"

class GameScene 
{
public:
    GameScene();

    void InitModel();
    void Render(const glm::vec3& cameraPos, bool useTexture);
    void Resize(int w, int h);

    static void SetCamera(float d);

private:
    std::vector<Entity> m_entities;
    unsigned int m_screenWidth;
    unsigned int m_screenHeight;
};