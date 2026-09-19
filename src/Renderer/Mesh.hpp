#pragma once

#include "Application/pch.hpp"
#include "Renderer/ShaderHelper.hpp"
#include "Renderer/AssetManager.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct MeshData {
    std::vector<GLfloat> vertices;
    std::vector<GLuint>  indices;
};

class Mesh
{
public:
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    static std::shared_ptr<Mesh> CreateSquare(float width, float height,
        const std::string& vertShaderFile, const std::string& fragShaderFile, const std::string& textureFile);

    static std::shared_ptr<Mesh> CreateTriangle(float width, float height,
        const std::string& vertShaderFile, const std::string& fragShaderFile, const std::string& textureFile);

    static std::shared_ptr<Mesh> CreateCircle(float radius, int segments,
        const std::string& vertShaderFile, const std::string& fragShaderFile, const std::string& textureFile);

    void Render(const glm::vec3& cameraPos, bool useTexture, int screenW, int screenH);

private:
    Mesh() = default;

    static std::shared_ptr<Mesh> Create(const MeshData& data,
        const std::string& vertShaderFile, const std::string& fragShaderFile, const std::string& textureFile);

    static MeshData GenerateSquareData(float width, float height);
    static MeshData GenerateTriangleData(float width, float height);
    static MeshData GenerateCircleData(float radius, int segments);

    AssetManager m_assetmanager;
    ShaderHelper m_shaderhelper;

    GLuint  m_VAO = 0, m_VBO = 0, m_EBO = 0;
    GLsizei m_indexCount = 0;

    GLuint m_shaderProgram = 0, m_texture = 0;
    GLint  m_locTex2d = -1, m_locView = -1, m_locProjection = -1, m_locUseTexture = -1;
};
