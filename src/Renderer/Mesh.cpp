#include "Renderer/Mesh.hpp"
#include "Application/Platform.hpp"

Mesh::~Mesh()
{
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
}

std::shared_ptr<Mesh> Mesh::CreateSquare(float width, float height)
{ 
    return Create(GenerateSquareData(width, height)); 
}

std::shared_ptr<Mesh> Mesh::CreateTriangle(float width, float height)
{ 
    return Create(GenerateTriangleData(width, height)); 
}

std::shared_ptr<Mesh> Mesh::CreateCircle(float radius, int segments)
{ 
    return Create(GenerateCircleData(radius, segments)); 
}

std::shared_ptr<Mesh> Mesh::Create(const MeshData& data)
{
    auto mesh = std::shared_ptr<Mesh>(new Mesh());

    glGenVertexArrays(1, &mesh->m_VAO);
    glBindVertexArray(mesh->m_VAO);

    glGenBuffers(1, &mesh->m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_VBO);
    glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(GLfloat), data.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(GLuint), data.indices.data(), GL_STATIC_DRAW);

    mesh->m_indexCount = static_cast<GLsizei>(data.indices.size());

    GLsizei stride = 5 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);

    LOGI("Mesh::Create done (VAO=%u, VBO=%u, EBO=%u)", mesh->m_VAO, mesh->m_VBO, mesh->m_EBO);
    return mesh;
}

void Mesh::Draw()
{
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

MeshData Mesh::GenerateSquareData(float width, float height)
{
    float hw = width * 0.5f, hh = height * 0.5f;
    return MeshData{
        { -hw,-hh,0.0f, 0.0f,0.0f,   -hw,hh,0.0f, 0.0f,1.0f,
           hw,-hh,0.0f, 1.0f,0.0f,    hw,hh,0.0f, 1.0f,1.0f },
        { 2, 0, 1, 3, 2, 1 }
    };
}

MeshData Mesh::GenerateTriangleData(float width, float height)
{
    float hw = width * 0.5f, hh = height * 0.5f;
    return MeshData{
        { -hw,-hh,0.0f, 0.0f,0.0f,   hw,-hh,0.0f, 1.0f,0.0f,   0.0f,hh,0.0f, 0.5f,1.0f },
        { 0, 1, 2 }
    };
}

MeshData Mesh::GenerateCircleData(float radius, int segments)
{
    MeshData data;
    data.vertices.insert(data.vertices.end(), { 0.0f,0.0f,0.0f, 0.5f,0.5f });
    
    for (int i = 0; i <= segments; ++i) {
        float angle = (static_cast<float>(i) / segments) * 2.0f * 3.14159265f;
        float x = radius * std::cos(angle), y = radius * std::sin(angle);
        data.vertices.insert(data.vertices.end(), { x, y, 0.0f, 0.5f*std::cos(angle)+0.5f, 0.5f*std::sin(angle)+0.5f });
    }

    for (int i = 1; i <= segments; ++i) { 
        data.indices.push_back(0); data.indices.push_back(i); data.indices.push_back(i+1); 
    }
    return data;
}