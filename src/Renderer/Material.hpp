#pragma once
#include "Application/pch.hpp"
#include "Renderer/ShaderHelper.hpp"
#include "Renderer/AssetManager.hpp"
#include "Application/Platform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Material;
using MaterialPtr = std::shared_ptr<Material>;

class Material
{
public:
    ~Material();
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    Material(Material&&) = delete;
    Material& operator=(Material&&) = delete;

    static MaterialPtr Create(const std::string& textureFile = "",
                                            const glm::vec4& color = glm::vec4(1.0f),
                                            bool useTexture = true,
                                            const std::string& vertShaderFile = "vertexshader.glsl",
                                            const std::string& fragShaderFile = "fragmentshader.glsl");

    void Bind(const glm::vec3& cameraPos, int screenW, int screenH, const glm::vec3& position, bool globalUseTexture);

private:
    Material() = default;

    GLuint    m_shaderProgram = 0, m_texture = 0;
    bool      m_useTexture = true;
    glm::vec4 m_color{1.0f};

    GLint m_locTex2d = -1, m_locView = -1, m_locProjection = -1,
          m_locModel = -1, m_locUseTexture = -1, m_locColor = -1;
};