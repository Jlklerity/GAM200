#pragma once
#include "Application/pch.hpp"

namespace ShaderHelper
{
    GLuint CompileShader(GLuint type, const std::string& source); 
    GLuint CreateShaderProgram(const std::string& vSource, const std::string& fSource); 

};
