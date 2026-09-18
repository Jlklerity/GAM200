#include "Player.hpp"
#include "Platform.hpp"
#include "AssetManager.hpp"
#include "ShaderHelper.hpp"
#include "pch.hpp"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const std::vector<GLfloat> vertexData{
        -0.5f, -0.5f,  0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.0f,   0.8f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,
    };
const std::vector<GLuint> indexData{ 2,0,1, 3,2,1 };

static constexpr GLuint ATTRIB_POSITION = 0;

Player::Player()
    : m_VAO{0}, m_VBO{0}, m_EBO{0}, m_indexCount{0},
      m_shaderProgram{0}, m_texture{0}, m_locTex2d{-1}, m_locProjection{-1},
      m_locUseTexture{-1}, m_useTexture{false}
{ 
    std::cout<<"Player made!"; 
}

Player::~Player()
{
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_texture) glDeleteTextures(1, &m_texture);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

void Player::InitModel()      
{
    LOGI("Player::InitModel");
    std::string vSource = m_assetmanager.LoadShaderAsString(m_assetmanager.ShaderPath("player_vertexshader.glsl"));
    std::string fSource = m_assetmanager.LoadShaderAsString(m_assetmanager.ShaderPath("player_fragmentshader.glsl"));
    
    m_shaderProgram = m_shaderhelper.CreateShaderProgram(vSource, fSource);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW);

    m_indexCount = static_cast<GLsizei>(indexData.size());
    GLsizei stride = 9 * sizeof(GLfloat);  
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat))); 
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(GLfloat)));
    glBindVertexArray(0);

    if (!m_shaderProgram) { LOGE("Player: failed to build shader program"); return; }
    m_locTex2d = glGetUniformLocation(m_shaderProgram, "uTex2d");
    m_locView = glGetUniformLocation(m_shaderProgram, "u_View");
    m_locProjection = glGetUniformLocation(m_shaderProgram, "u_Projection");
    m_locUseTexture = glGetUniformLocation(m_shaderProgram, "u_useTexture");
    m_texture = m_assetmanager.setup_texobj(m_assetmanager.ImagePath("player.png"));

    
    if (m_locTex2d >= 0) 
    {
        glUseProgram(m_shaderProgram);
        glUniform1i(m_locTex2d, 0);
        glUseProgram(0);
    }

     LOGI("Player::InitModel done (VAO=%u, VBO=%u, EBO=%u)", m_VAO, m_VBO, m_EBO);
}

void Player::Render(const glm::vec3& cameraPos, bool useTexture)
{
    render_Player(cameraPos, useTexture);
}

void Player::Resize(int w, int h)
{
    m_width = w;
    m_height = h;
}

void Player::render_Player(const glm::vec3& cameraPos, bool useTexture)
{
    glUseProgram(m_shaderProgram);

    glm::mat4 projection{};
    float aspect = static_cast<float>(m_width) / static_cast<float>(m_height > 0 ? m_height : 1);

    projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

    glm::mat4 view = glm::translate(glm::mat4(1.0f), -cameraPos);
        

    if (m_locProjection >= 0) glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(projection));
    if (m_locView >= 0) glUniformMatrix4fv(m_locView, 1, GL_FALSE, glm::value_ptr(view));
    if (m_locUseTexture >= 0) glUniform1i(m_locUseTexture, useTexture ? 1 : 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);      
    glUseProgram(0);
}
