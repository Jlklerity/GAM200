#pragma once

#include "Platform.hpp"

// --- Third party -----------------------------------------------------------
#define STBI_NO_STDIO      // no fopen path: decoding happens from memory only
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG

#if defined(_MSC_VER)
#pragma warning(push, 0)   // stb is noisy at /W4
#endif
#include "stb_image.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// --- Standard library ------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>