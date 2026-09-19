#pragma once

/**
 * Platform.h
 *
 * Single-header platform detection for the OpenGLES3 Primitives demo.
 * Supports three targets:
 *   PLATFORM_EMSCRIPTEN  – WebGL 2.0 via Emscripten + SDL2 (-s USE_SDL=2)
 *   PLATFORM_WINDOWS     – OpenGL ES 3.0 via SDL2 + glad (gles2=3.0)
 *   PLATFORM_ANDROID     – OpenGL ES 3.0 via SDL2 + Android NDK
 *
 * IMPORTANT: Emscripten must be detected BEFORE Windows because emcc
 * defines both __EMSCRIPTEN__ AND _WIN32 on some toolchain setups.
 */

// ---------------------------------------------------------------------------
// Platform detection
// ---------------------------------------------------------------------------
#ifdef __EMSCRIPTEN__
    #ifndef PLATFORM_EMSCRIPTEN
        #define PLATFORM_EMSCRIPTEN
    #endif
#elif defined(_WIN32)
    #ifndef PLATFORM_WINDOWS
        #define PLATFORM_WINDOWS
    #endif
#endif
// ---------------------------------------------------------------------------
// Platform-specific OpenGL and system headers
// ---------------------------------------------------------------------------
#ifdef PLATFORM_EMSCRIPTEN
    // WebGL2 == ES 3.0. Emscripten links the ES symbols directly, no loader.
    #include <GLES3/gl3.h>
    #include <SDL2/SDL.h>
    #include <emscripten.h>
    #include <cstdio>
    #include <cstring>

    #define LOGI(...) do { printf("[INFO] "  __VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] " __VA_ARGS__); printf("\n"); } while(0)

#elif defined(PLATFORM_WINDOWS)
    // Desktop has no ES symbols of its own: glad supplies the ES 3.0 entry
    // points, resolved at runtime through SDL_GL_GetProcAddress.
    #include <glad/gles2.h>
    #include <SDL.h>
    #include <cstdio>
    #include <cstring>

    #define PLATFORM_NEEDS_GL_LOADER 1

    #define LOGI(...) do { printf("[INFO] "  __VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] " __VA_ARGS__); printf("\n"); } while(0)

#else
    #error "Platform.h: unsupported target (expected Emscripten or Windows)"
#endif
