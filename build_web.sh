#!/usr/bin/env sh
# =============================================================================
# build_web.sh - Build GAM200 for WebGL2 (OpenGL ES 3.0) via Emscripten + SDL2
#
# Prerequisites:
#   1. Install the Emscripten SDK:  https://emscripten.org/docs/getting_started/
#   2. Activate it in your shell:   source /path/to/emsdk/emsdk_env.sh
#   3. Run this script from the project root:  ./build_web.sh
#
# Output: web/index.html  (+ index.js, index.wasm, index.data)
# Serve:  cd web && python3 -m http.server 8080
# Open:   http://localhost:8080/
#
# Backend: SDL2 only. The GLFW path is gone -- Platform.h and main.cpp are
# SDL2-only now, so there is no second block to uncomment.
# =============================================================================

set -e

command -v em++ >/dev/null 2>&1 || {
    echo "error: em++ not found on PATH."
    echo "       Run: source /path/to/emsdk/emsdk_env.sh"
    exit 1
}

mkdir -p web

# --- Fetch GLM (same tag the desktop build pulls via CMake FetchContent) -----
GLM_DIR="third_party/glm"
GLM_TAG="1.0.1"

if [ ! -d "$GLM_DIR" ]; then
    echo "Fetching GLM ${GLM_TAG} into ${GLM_DIR} ..."
    git clone --branch "$GLM_TAG" --depth 1 https://github.com/g-truc/glm.git "$GLM_DIR"
fi


STB_DIR="third_party/stb"

if [ ! -d "$STB_DIR" ]; then
    echo "Fetching stb into ${STB_DIR} ..."
    git clone --depth 1 https://github.com/nothings/stb.git "$STB_DIR"
fi

# --- Build ------------------------------------------------------------------
# Flag notes:
#
#   -sUSE_SDL=2
#       SDL2 port. (--use-port=sdl2 is the newer spelling; both are current.)
#
#   -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
#       WebGL2 IS OpenGL ES 3.0. Replaces -sUSE_WEBGL2=1, which Emscripten now
#       marks deprecated. MIN=2 drops the WebGL1 fallback, which is useless to
#       us: there is no automatic downgrade, and ES 3.0 needs WebGL2 anyway.
#
#   FULL_ES3 is deliberately NOT set. It emulates the GLES3 features WebGL2
#       lacks -- chiefly client-side vertex arrays -- and pulls in FULL_ES2,
#       costing code size and speed. main.cpp draws only from VBOs, so it is
#       not needed. Add it back if you hit an unsupported GLES3 path later.
#
#   --preload-file shaders
#       Packs shaders/ into index.data, mounted at /shaders in MEMFS.
#       main.cpp opens "../../shaders/...", and Emscripten's working directory
#       is "/", so that normalises to "/shaders/..." because paths above the
#       root get clamped. That works, but it is an accident of path handling --
#       see the note in the build output below.
#
#   WASM=1 is the default now, so it is no longer passed explicitly.

# em++, NOT emcc. Emscripten links libc++/libc++abi only when LINK_AS_CXX is
# set, and that is driven by the driver name (tools/link.py: run_via_emxx).
# emcc compiles .cpp as C++ but does not link the C++ standard library, which
# shows up as undefined std::/__cxa_/operator new symbols at link time.
# (-sDEFAULT_TO_CXX=1 would also work; em++ is the documented way.)
em++ -std=c++20 -O2 \
    Scene/main.cpp \
    Scene/Application.cpp \
    Scene/AssetManager.cpp \
    Scene/InputManager.cpp \
    Scene/stb_image_impl.cpp \
    -IScene \
    -I"$GLM_DIR" \
    -I"$STB_DIR" \
    -sUSE_SDL=2 \
    -sMIN_WEBGL_VERSION=2 \
    -sMAX_WEBGL_VERSION=2 \
    -sALLOW_MEMORY_GROWTH=1 \
    --preload-file shaders \
    --preload-file images \
    -o web/index.html

# For a debug build, swap -O2 above for:  -O0 -g -gsource-map -sASSERTIONS=2

echo ""
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"
