#pragma once
#include "Application/Platform.hpp"
#include <string>

class ShaderHelper
{
public:
    GLuint CompileShader(GLuint type, const std::string& source) 
    {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        return shader;
    }
    
    GLuint CreateShaderProgram(const std::string& vSource, const std::string& fSource) 
    {
        GLuint program = glCreateProgram();
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vSource);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fSource);
    
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
    
        glDetachShader(program, vs);
        glDetachShader(program, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return program;
    }
};
