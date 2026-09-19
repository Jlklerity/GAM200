#pragma once
#include <vector>
#include "Scene/Model.hpp"
#include "Scene/Entity.hpp"
#include "Application/Platform.hpp"

class GameScene : public Model
{
public:
    GameScene();

    void InitModel()            override;
    void Render(const glm::vec3& cameraPos, bool useTexture) override;
    void Resize(int w, int h)   override;

    static void SetCamera(float d);

private:
    std::vector<Entity> m_entities;
    unsigned int m_screenWidth;
    unsigned int m_screenHeight;
};