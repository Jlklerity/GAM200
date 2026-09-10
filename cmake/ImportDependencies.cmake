# ===========================================================================
# ImportDependencies.cmake
#
# Downloads SDL2 + glad (OpenGL ES 3.0 loader) for the desktop build.
# Android and Emscripten have built-in GL support and skip this.
# ===========================================================================

include(FetchContent)

function(importDependencies)

    if(ANDROID)
        return()
    endif()

    # SDL2 -- NOT "master", which is SDL3 now. Verified tag: release-2.32.10
    message(STATUS "[ImportDependencies] Fetching SDL2 ...")
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-2.32.10
        GIT_SHALLOW    TRUE
    )
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON  CACHE BOOL "" FORCE)
    set(SDL_TEST   OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL2)

    # glad2 generates the OpenGL ES 3.0 loader. Needs Python on PATH, and
    # network access on the first configure (it downloads the Khronos specs).
    message(STATUS "[ImportDependencies] Fetching glad2 ...")
    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG        v2.0.8
        GIT_SHALLOW    TRUE
        SOURCE_SUBDIR  cmake
    )
    FetchContent_MakeAvailable(glad)

    # Creates target 'glad_gles2' exposing <glad/gles2.h> and gladLoadGLES2().
    glad_add_library(glad_gles2 REPRODUCIBLE API gles2=3.0)
	
    message(STATUS "[ImportDependencies] Fetching GLM ...")
    FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(glm)
    message(STATUS "[ImportDependencies] All desktop dependencies ready.")

endfunction()
