#include "Scene/GameScene.hpp"
#include "Renderer/Mesh.hpp"
#include "Input/InputManager.hpp"
#include "Renderer/ShaderHelper.hpp"
#include <iostream>

GameScene::GameScene()
    : m_screenWidth{0}, m_screenHeight{0}
{
}

void GameScene::InitModel()
{
    MeshPtr squareMesh     = Mesh::CreateSquare(1.0f, 1.0f);
    MeshPtr floorMesh     = Mesh::CreateSquare(0.1f, 0.1f);
    MeshPtr circleMesh12     = Mesh::CreateCircle(1.0f, 12);
    MeshPtr circleMesh25     = Mesh::CreateCircle(1.0f, 25);
    MeshPtr triangleMesh   = Mesh::CreateTriangle(1.0f, 1.0f);
    
    
   
    // MaterialPtr playerMaterial = Material::Create("player.png");
    // if (squareMesh && playerMaterial)
    //     m_entities.emplace_back(squareMesh, playerMaterial, glm::vec3(0.0f));
        
    MaterialPtr floorMaterial = Material::Create("floor_background.jpg");
    if (squareMesh && floorMaterial)
    {
        float x_pos{-1.0f}, y_pos{0.0f};
        for (int y = 0; y < 10; ++y)
        {
            for (int x = 0; x < 10; ++x)
            {
                m_entities.emplace_back(floorMesh, floorMaterial, glm::vec3(x_pos, y_pos,0.0f));
                x_pos += 0.1f;
            }
            x_pos = -1.0f;
            y_pos += 0.1f; 
        }
    }


    // MaterialPtr spikeMaterial = Material::Create("spike.jpg");
    // if (triangleMesh && spikeMaterial)
    //     m_entities.emplace_back(triangleMesh, spikeMaterial, glm::vec3(0.5f, -0.5f, 0.1f));
    
    // m_entities.emplace_back(triangleMesh, spikeMaterial, glm::vec3(-0.5f, -0.5f, 0.1f));
    // m_entities.emplace_back(circleMesh25, playerMaterial, glm::vec3(-0.9f, -0.9f, 0.1f));
    

}

void GameScene::Render(const glm::vec3& cameraPos, bool useTexture)
{
    // if (InputManager::IsKeyPressed(Key::Up))
    //     m_entities[3].setPosition(glm::vec3{0.1f, 0.1f, 0.0f});

    // if (InputManager::IsKeyPressed(Key::G))
    //     m_entities[4].setPosition(glm::vec3{0.1f, 0.0f, 0.0f});


       
    // if (InputManager::IsKeyPressed(Key::Down))
    //     m_entities[3].setPosition(glm::vec3{-0.1f, -0.1f, 0.0f});

    for (auto& entity : m_entities)
    {
        entity.Render(cameraPos, static_cast<int>(m_screenWidth), static_cast<int>(m_screenHeight), useTexture);
        //std::cout << entity.getPosition()<< '\n';
    }
}

void GameScene::SetCamera(float d) { }

void GameScene::Resize(int w, int h)
{
    m_screenWidth  = static_cast<unsigned int>(w);
    m_screenHeight = static_cast<unsigned int>(h);
}