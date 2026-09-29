#pragma once
#include "Engine.hpp"
#include "Entity.hpp"
#include <vector>

class GameScene : public Scene
{
public:
    GameScene();

    void InitModel() override;
    void Update(float dt) override;
    void Draw() override;
    void Resize(int w, int h) override;
    void CleanUp() override;

    static void SetCamera(float d);

private:
    std::vector<Entity> m_entities;
    unsigned int m_screenWidth;
    unsigned int m_screenHeight;

    glm::vec3    m_cameraPos;
    bool         m_useTexture;
};