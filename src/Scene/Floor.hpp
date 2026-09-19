#pragma once

#include "Scene/Model.hpp"
#include "Renderer/AssetManager.hpp"
#include "Renderer/ShaderHelper.hpp"
#include "Renderer/Mesh.hpp"

class Floor : public Model
{
public:
    Floor() = default;
    ~Floor() override = default;

    void InitModel()            override;
    void Render(const glm::vec3& cameraPos, bool useTexture) override;
    void Resize(int w, int h)   override;

private:
    std::shared_ptr<Mesh> m_mesh;
    int m_width  = 0;
    int m_height = 0;
};