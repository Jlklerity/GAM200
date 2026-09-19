#include "Scene/GameScene.hpp"

GameScene::GameScene()
    : m_screenWidth{0}, m_screenHeight{0}
{
}

void GameScene::InitModel()
{
    auto playerMesh     = Mesh::CreateSquare(1.0f, 1.0f);
    auto playerMaterial = Material::Create("player.png");
    if (playerMesh && playerMaterial)
        m_entities.emplace_back(playerMesh, playerMaterial, glm::vec3(0.0f));
        
    auto floorMesh     = Mesh::CreateCircle(1.0f, 12);
    auto floorMaterial = Material::Create("floor_background.jpg");
    if (floorMesh && floorMaterial)
        m_entities.emplace_back(floorMesh, floorMaterial, glm::vec3(0.0f));

}

void GameScene::Render(const glm::vec3& cameraPos, bool useTexture)
{
    for (auto& entity : m_entities)
        entity.Render(cameraPos, static_cast<int>(m_screenWidth), static_cast<int>(m_screenHeight), useTexture);
}

void GameScene::SetCamera(float d) { }

void GameScene::Resize(int w, int h)
{
    m_screenWidth  = static_cast<unsigned int>(w);
    m_screenHeight = static_cast<unsigned int>(h);
}