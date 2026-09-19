#pragma once

#include <memory>
#include "Scene/Model.hpp"
#include "Application/Platform.hpp"
#include "Scene/Player.hpp"
#include "Scene/Floor.hpp"

class GameScene : public Model 
{
public:
    GameScene();

    void InitModel()            override;
    void Render(const glm::vec3& cameraPos, bool useTexture) override;
    void Resize(int w, int h)   override;

    static void SetCamera(float d);

private:
    std::unique_ptr<Player> m_player;
    std::unique_ptr<Floor> m_floor;

    unsigned int m_screenWidth;
    unsigned int m_screenHeight;
};

