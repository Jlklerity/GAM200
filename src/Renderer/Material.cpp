#include "Renderer/Material.hpp"
#include "Application/Platform.hpp"

Material::~Material()
{
    if (m_texture) glDeleteTextures(1, &m_texture);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

std::shared_ptr<Material> Material::Create(const std::string& textureFile, const glm::vec4& color,
    bool useTexture, const std::string& vertShaderFile, const std::string& fragShaderFile)
{
    auto mat = std::shared_ptr<Material>(new Material());
    mat->m_color = color;
    mat->m_useTexture = useTexture;

    std::string vSource = mat->m_assetmanager.LoadShaderAsString(mat->m_assetmanager.ShaderPath(vertShaderFile));
    std::string fSource = mat->m_assetmanager.LoadShaderAsString(mat->m_assetmanager.ShaderPath(fragShaderFile));
    if (vSource.empty() || fSource.empty()) {
        LOGE("Material::Create: shader source missing (vert='%s', frag='%s')", vertShaderFile.c_str(), fragShaderFile.c_str());
        return nullptr;
    }

    mat->m_shaderProgram = mat->m_shaderhelper.CreateShaderProgram(vSource, fSource);
    if (!mat->m_shaderProgram) {
        LOGE("Material::Create: failed to build shader program");
        return nullptr;
    }

    mat->m_locTex2d      = glGetUniformLocation(mat->m_shaderProgram, "uTex2d");
    mat->m_locView       = glGetUniformLocation(mat->m_shaderProgram, "u_View");
    mat->m_locProjection = glGetUniformLocation(mat->m_shaderProgram, "u_Projection");
    mat->m_locModel      = glGetUniformLocation(mat->m_shaderProgram, "u_Model");
    mat->m_locUseTexture = glGetUniformLocation(mat->m_shaderProgram, "u_useTexture");
    mat->m_locColor      = glGetUniformLocation(mat->m_shaderProgram, "u_Color");

    if (useTexture && !textureFile.empty())
        mat->m_texture = mat->m_assetmanager.setup_texobj(mat->m_assetmanager.ImagePath(textureFile));

    if (mat->m_locTex2d >= 0) {
        glUseProgram(mat->m_shaderProgram);
        glUniform1i(mat->m_locTex2d, 0);
        glUseProgram(0);
    }
    return mat;
}

void Material::Bind(const glm::vec3& cameraPos, int screenW, int screenH, const glm::vec3& position, bool globalUseTexture)
{
    glUseProgram(m_shaderProgram);

    float aspect = static_cast<float>(screenW) / static_cast<float>(screenH > 0 ? screenH : 1);
    glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 view  = glm::translate(glm::mat4(1.0f), -cameraPos);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);

    if (m_locProjection >= 0) glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(projection));
    if (m_locView       >= 0) glUniformMatrix4fv(m_locView,       1, GL_FALSE, glm::value_ptr(view));
    if (m_locModel      >= 0) glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(model));
    if (m_locUseTexture >= 0) glUniform1i(m_locUseTexture, m_useTexture ? 1 : 0);
    if (m_locColor      >= 0) glUniform4fv(m_locColor, 1, glm::value_ptr(m_color));

    bool effectiveUseTexture = m_useTexture && globalUseTexture;   
    if (m_locUseTexture >= 0) glUniform1i(m_locUseTexture, effectiveUseTexture ? 1 : 0);
    if (effectiveUseTexture && m_texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }
}