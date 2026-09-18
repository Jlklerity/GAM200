#pragma once

#include "Model.hpp"
#include "AssetManager.hpp"
#include "ShaderHelper.hpp"

class Player : public Model 
{
public:
    Player();

    ~Player() override;

    void InitModel()            override;
    void Render(const glm::vec3& cameraPos, bool useTexture) override;    
    void Resize(int w, int h)   override;
    void UpdateCamera(float distance);

private:
    void render_Player(const glm::vec3& cameraPos, bool useTexture);
    AssetManager m_assetmanager;
    ShaderHelper m_shaderhelper;

    unsigned int m_program = 0;
    // Geometry
    GLuint  m_VAO;
    GLuint  m_VBO;
    GLuint  m_EBO;
    GLsizei m_indexCount;

    // Shader / Textures
    GLuint m_shaderProgram;
    GLuint m_texture;
    GLint  m_locTex2d;
    GLint  m_locView;
    GLint  m_locProjection;
    GLint  m_locUseTexture;
    bool   m_useTexture;
    GLint  m_width;
    GLint  m_height; 

    GLsizeiptr posSize   = 6 * 3 * sizeof(GLfloat);   
};