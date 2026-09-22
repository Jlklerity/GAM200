#pragma once
#include "Application/pch.hpp"

namespace AssetManager
{
    std::string AssetPath(const std::string& folder, const std::string& name);
    std::string ShaderPath(const std::string& name);
    std::string ImagePath(const std::string& name);
    std::string LoadShaderAsString(const std::string& path);
    GLuint setup_texobj(std::string const& tex_path);
}