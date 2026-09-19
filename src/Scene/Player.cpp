// Player.cpp
#include "Scene/Player.hpp"
#include "Application/Platform.hpp"

void Player::InitModel()
{

    m_mesh = Mesh::CreateSquare(1.0f, 1.0f, "player_vertexshader.glsl", 
                                "player_fragmentshader.glsl", "player.png");
    if (!m_mesh) {
        LOGE("Player::InitModel: failed to build mesh");
    }
}

void Player::Render(const glm::vec3& cameraPos, bool useTexture)
{
    if (m_mesh) m_mesh->Render(cameraPos, useTexture, m_width, m_height);
}

void Player::Resize(int w, int h)
{
    m_width  = w;
    m_height = h;
}