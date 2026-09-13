#pragma once
#include "pch.hpp"
#include <string>


class AssetManager
{
public:
    static std::string AssetPath(const std::string& folder, const std::string& name);
    static std::string ShaderPath(const std::string& name);
    static std::string ImagePath(const std::string& name);
    static std::string LoadShaderAsString(const std::string& path);
    static GLuint setup_texobj(std::string const& tex_path);
};