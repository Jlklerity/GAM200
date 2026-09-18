#include "GameScene.hpp"
#include <glm/gtc/matrix_transform.hpp>

GameScene::GameScene()
    : m_screenWidth{0}, m_screenHeight{0}
{
    m_player = std::make_unique<Player>();
    m_floor = std::make_unique<Floor>();
}

void GameScene::InitModel()
{
    if(!m_player || !m_floor) return;

    m_player->InitModel();
    m_floor->InitModel();
}

void GameScene::Render(const glm::vec3& cameraPos, bool useTexture)
{
    if(m_player) m_player->Render(cameraPos, useTexture);
    if(m_floor) m_floor->Render(cameraPos, useTexture);    
     
}

void GameScene::SetCamera(float d)
{

}

void GameScene::Resize(int w, int h)
{
    m_screenWidth = w;
    m_screenHeight = h;
    
    if (m_player) m_player->Resize(w, h);
    if (m_floor) m_floor->Resize(w, h);

}