#include "Mesh.hpp"

Mesh::~Mesh()
{
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_texture) glDeleteTextures(1, &m_texture);
        if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

std::shared_ptr<Mesh> Mesh::CreateSquare(float width, float height,
        const std::string& vertShaderFile, const std::string& fragShaderFile, const std::string& textureFile)
{
    return Create(GenerateSquareData(width, height), vertShaderFile, fragShaderFile, textureFile);
}

std::shared_ptr<Mesh> Mesh::CreateTriangle(float width, float height,
        const std::string& vertShaderFile, const std::string& fragShaderFile, const std::string& textureFile)
{
    return Create(GenerateTriangleData(width, height), vertShaderFile, fragShaderFile, textureFile);
}

std::shared_ptr<Mesh> Mesh::CreateCircle(float radius, int segments,
        const std::string& vertShaderFile, const std::string& fragShaderFile, const std::string& textureFile)
{
    return Create(GenerateCircleData(radius, segments), vertShaderFile, fragShaderFile, textureFile);
}

std::shared_ptr<Mesh> Mesh::Create(const MeshData& data,
                                    const std::string& vertShaderFile,
                                    const std::string& fragShaderFile,
                                    const std::string& textureFile)
{
    auto mesh = std::shared_ptr<Mesh>(new Mesh());

    std::string vSource = mesh->m_assetmanager.LoadShaderAsString(mesh->m_assetmanager.ShaderPath(vertShaderFile));
    std::string fSource = mesh->m_assetmanager.LoadShaderAsString(mesh->m_assetmanager.ShaderPath(fragShaderFile));
    mesh->m_shaderProgram = mesh->m_shaderhelper.CreateShaderProgram(vSource, fSource);

    glGenVertexArrays(1, &mesh->m_VAO);
    glBindVertexArray(mesh->m_VAO);

    glGenBuffers(1, &mesh->m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_VBO);
    glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(GLfloat), data.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(GLuint), data.indices.data(), GL_STATIC_DRAW);

    mesh->m_indexCount = static_cast<GLsizei>(data.indices.size());
    GLsizei stride = 9 * sizeof(GLfloat);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(GLfloat)));
    glBindVertexArray(0);

    if (!mesh->m_shaderProgram) {
        LOGE("Mesh::Create: failed to build shader program");
        return nullptr;
    }

    mesh->m_locTex2d      = glGetUniformLocation(mesh->m_shaderProgram, "uTex2d");
    mesh->m_locView       = glGetUniformLocation(mesh->m_shaderProgram, "u_View");
    mesh->m_locProjection = glGetUniformLocation(mesh->m_shaderProgram, "u_Projection");
    mesh->m_locUseTexture = glGetUniformLocation(mesh->m_shaderProgram, "u_useTexture");
    mesh->m_texture       = mesh->m_assetmanager.setup_texobj(mesh->m_assetmanager.ImagePath(textureFile));

    if (mesh->m_locTex2d >= 0) {
        glUseProgram(mesh->m_shaderProgram);
        glUniform1i(mesh->m_locTex2d, 0);
        glUseProgram(0);
    }

    LOGI("Mesh::Create done (VAO=%u, VBO=%u, EBO=%u)", mesh->m_VAO, mesh->m_VBO, mesh->m_EBO);
    return mesh;
}

void Mesh::Render(const glm::vec3& cameraPos, bool useTexture, int screenW, int screenH)
{
    glUseProgram(m_shaderProgram);

    float aspect = static_cast<float>(screenW) / static_cast<float>(screenH > 0 ? screenH : 1);
    glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), -cameraPos);

    if (m_locProjection >= 0) glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(projection));
    if (m_locView >= 0)       glUniformMatrix4fv(m_locView, 1, GL_FALSE, glm::value_ptr(view));
    if (m_locUseTexture >= 0) glUniform1i(m_locUseTexture, useTexture ? 1 : 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

MeshData Mesh::GenerateSquareData(float width, float height)
{
    float hw = width  * 0.5f;
    float hh = height * 0.5f;

    return MeshData{
        {
            -hw, -hh, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  // bottom-left
            -hw,  hh, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f,  // top-left
             hw, -hh, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,  // bottom-right
             hw,  hh, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 1.0f,  // top-right
        },
        { 2, 0, 1, 3, 2, 1 }   
    };
}

MeshData Mesh::GenerateTriangleData(float width, float height)
{
    float hw = width  * 0.5f;
    float hh = height * 0.5f;

    return MeshData{
        {
           -hw, -hh, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  // bottom-left
            hw, -hh, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,  // bottom-right
            0.0f, hh, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,   0.5f, 1.0f,  // top-center
        },
        { 0, 1, 2 }
    };
}

MeshData Mesh::GenerateCircleData(float radius, int segments)
{
    MeshData data;

    data.vertices.insert(data.vertices.end(), {
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.5f, 0.5f
    });

    for (int i = 0; i <= segments; ++i)
    {
        float t     = static_cast<float>(i) / static_cast<float>(segments);
        float angle = t * 2.0f * 3.14159265f;
        float x     = radius * std::cos(angle);
        float y     = radius * std::sin(angle);

        data.vertices.insert(data.vertices.end(), {
            x, y, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.5f * std::cos(angle) + 0.5f, 0.5f * std::sin(angle) + 0.5f
        });
    }

    for (int i = 1; i <= segments; ++i)
    {
        data.indices.push_back(0);
        data.indices.push_back(i);
        data.indices.push_back(i + 1);
    }

    return data;
}