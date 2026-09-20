#pragma once
#include "Application/pch.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"
#include <iostream>
class Entity
{
public:
    Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const glm::vec3& position)
        : m_mesh(std::move(mesh)), m_material(std::move(material)), m_position(position) {}

    void Render(const glm::vec3& cameraPos, int screenW, int screenH, bool useTexture)
    {
        if (!m_mesh || !m_material) return;
        m_material->Bind(cameraPos, screenW, screenH, m_position, useTexture);
        m_mesh->Draw();
    }

    const glm::vec3& getPosition() const 
    { 
        return m_position; 
    }

private:
    std::shared_ptr<Mesh>     m_mesh;
    std::shared_ptr<Material> m_material;
    glm::vec3                 m_position;
};

inline std::ostream& operator<<(std::ostream& os, const glm::vec3& pos)
{
    os << "(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
    return os;
}