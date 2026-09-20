#pragma once
#include "Application/pch.hpp"

struct MeshData {
    std::vector<GLfloat> vertices;   
    std::vector<GLuint>  indices;
};

class Mesh;
using MeshPtr = std::shared_ptr<Mesh>;

class Mesh
{
public:
    
    
    ~Mesh();
    Mesh() = delete;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    static MeshPtr CreateSquare(float width, float height);
    static MeshPtr CreateTriangle(float width, float height);
    static MeshPtr CreateCircle(float radius, int segments);

    void Draw();

private:
    static MeshPtr Create(const MeshData& data);

    static MeshData GenerateSquareData(float width, float height);
    static MeshData GenerateTriangleData(float width, float height);
    static MeshData GenerateCircleData(float radius, int segments);

    GLuint  m_VAO = 0, m_VBO = 0, m_EBO = 0;
    GLsizei m_indexCount = 0;
};