// Floor.cpp
#include "Scene/Floor.hpp"
#include "Application/Platform.hpp"

void Floor::InitModel()
{
    m_mesh = Mesh::CreateCircle(1.0f, 12, "floor_vertexshader.glsl",
                                     "floor_fragmentshader.glsl", "floor_background.jpg");
    if (!m_mesh) {
        LOGE("Floor::InitModel: failed to build mesh");
    }
}

void Floor::Render(const glm::vec3& cameraPos, bool useTexture)
{
    if (m_mesh) m_mesh->Render(cameraPos, useTexture, m_width, m_height);
}

void Floor::Resize(int w, int h)
{
    m_width  = w;
    m_height = h;
}