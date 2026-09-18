#pragma once
#include "pch.hpp"

class Model
{
public:
    virtual ~Model() {}

    virtual void InitModel()          = 0;
    virtual void Render(const glm::vec3& cameraPos, bool useTexture) = 0;
    virtual void Resize(int w, int h) = 0;
        
};