#pragma once
#include "Application/pch.hpp"
#include <string>

class AssetManager
{
public:
    std::string AssetPath(const std::string& folder, const std::string& name)
    {
        static const std::string base = []() -> std::string
        {
            char* p = SDL_GetBasePath();
            if (p == nullptr)
            {
                return {};              // fall back to the CWD
            }
            std::string s{p};
            SDL_free(p);                // SDL_GetBasePath allocates; we own it
            return s;
        }();
        return base + folder + "/" + name;
    }

    std::string ShaderPath(const std::string& name)
    {
        return AssetPath("shaders", name);
    }

    std::string ImagePath(const std::string& name)
    {
        return AssetPath("images",  name);
    }

    std::string LoadShaderAsString(const std::string& path)
    {
        SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "rb");
        if (rw == nullptr)
        {
            LOGE("LoadShaderAsString: cannot open '%s': %s", path.c_str(), SDL_GetError());
            return {};
        }

        const Sint64 size = SDL_RWsize(rw);
        if (size <= 0)
        {
            LOGE("LoadShaderAsString: '%s' is empty or unsized: %s", path.c_str(), SDL_GetError());
            SDL_RWclose(rw);
            return {};
        }

        std::string result(static_cast<size_t>(size), '\0');
        const size_t got = SDL_RWread(rw, result.data(), 1, static_cast<size_t>(size));
        SDL_RWclose(rw);

        if (got != static_cast<size_t>(size))
        {
            LOGE("LoadShaderAsString: short read on '%s' (%lld of %lld bytes)",
                path.c_str(), (long long)got, (long long)size);
            return {};
        }

        return result;
    }

    GLuint setup_texobj(std::string const& tex_path)
    {
        SDL_RWops* rw = SDL_RWFromFile(tex_path.c_str(), "rb");
        if (rw == nullptr)
        {
            LOGE("setup_texobj: failed to open texture file '%s': %s",
                tex_path.c_str(), SDL_GetError());
            return 0;
        }

        const Sint64 size = SDL_RWsize(rw);
        if (size <= 0)
        {
            LOGE("setup_texobj: '%s' is empty or unsized: %s",
                tex_path.c_str(), SDL_GetError());
            SDL_RWclose(rw);
            return 0;
        }

        std::vector<stbi_uc> encoded(static_cast<size_t>(size));
        const size_t got = SDL_RWread(rw, encoded.data(), 1, encoded.size());
        SDL_RWclose(rw);

        if (got != encoded.size())
        {
            LOGE("setup_texobj: short read on '%s' (%lld of %lld bytes)",
                tex_path.c_str(), (long long)got, (long long)encoded.size());
            return 0;
        }

        // GL samples with the origin at the bottom-left; PNG and JPEG both store
        // the top row first. The .tex pipeline baked this flip in at conversion
        // time -- with encoded formats it has to happen at load.
        stbi_set_flip_vertically_on_load(1);

        int width = 0, height = 0, channels_in_file = 0;
        // The trailing 4 forces RGBA whatever the file holds, so the upload format
        // is fixed and a 3-channel image with an odd width cannot trip the default
        // GL_UNPACK_ALIGNMENT of 4 and shear diagonally.
        stbi_uc* ptr_texels = stbi_load_from_memory(
            encoded.data(), (static_cast<int>(encoded.size())),
            &width, &height, &channels_in_file, 4);

        if (ptr_texels == nullptr)
        {
            LOGE("setup_texobj: decode failed for '%s': %s",
                tex_path.c_str(), stbi_failure_reason());
            return 0;
        }

        GLuint texobj_hdl{};
        glGenTextures(1, &texobj_hdl);
        glBindTexture(GL_TEXTURE_2D, texobj_hdl);

        // allocate GPU storage for texture image data loaded from file
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
        // copy image data from client memory to GPU texture buffer memory
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGBA, GL_UNSIGNED_BYTE, ptr_texels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);
        // client memory not required since image is buffered in GPU memory
        stbi_image_free(ptr_texels);

        LOGI("setup_texobj: '%s' %dx%d (%d channels in file)",
            tex_path.c_str(), width, height, channels_in_file);
        return texobj_hdl;
    }
};