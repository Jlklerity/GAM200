#!/usr/bin/env sh
# =============================================================================
# build_web.sh - Build GAM200 for WebGL2 (OpenGL ES 3.0) via Emscripten + SDL2
#
# macOS/Linux counterpart of build_web.bat. Keep the two in sync.
#
# Prerequisites:
#   None you need to install by hand. If em++ is not already on PATH, this
#   script will bootstrap emsdk into ./emsdk itself (see below). If you'd
#   rather manage emsdk yourself, activate it before running this script:
#       source /path/to/emsdk/emsdk_env.sh
#
# Run this script from the project root:  ./build_web.sh
#
# Output: web/index.html  (+ index.js, index.wasm, index.data)
# Run:    emrun web/index.html
#         (emrun ships with emsdk and is on PATH once emsdk_env.sh has been
#         sourced -- either by you, or by the bootstrap step below. It starts
#         a local server, opens your default browser, and forwards the page's
#         console output back into this terminal, which python3 -m
#         http.server never did.)
#
# Backend: SDL2 only. The GLFW path is gone -- Platform.h and main.cpp are
# SDL2-only now, so there is no second block to uncomment.
# =============================================================================

set -e

# --- Locate or bootstrap em++ ------------------------------------------------
# emsdk is not a normal library dependency: it is an entire toolchain (its own
# clang, its own linker, its own copy of Python for emrun/glad-style helper
# scripts), so "just fetch a git repo" is not enough on its own -- it also has
# to be installed (fetches the actual prebuilt binaries for this OS/arch) and
# activated (writes .emscripten config + PATH/env additions).
#
# EMSDK_VERSION is pinned, not "latest", so a build from a fresh checkout
# today reproduces the same toolchain a build six months from now would use.
# Bump it deliberately, in its own commit, when you want to move -- keep this
# in sync with the version the README tells desktop contributors to install.
EMSDK_VERSION="6.0.9"

if ! command -v em++ >/dev/null 2>&1; then
    if [ -f "./emsdk/emsdk_env.sh" ]; then
        echo "Activating existing emsdk ..."
        . ./emsdk/emsdk_env.sh
    else
        echo "emsdk not found -- bootstrapping emsdk ${EMSDK_VERSION} into ./emsdk ..."
        git clone https://github.com/emscripten-core/emsdk.git || {
            echo "error: emsdk clone failed. Is git installed and on PATH?"
            exit 1
        }
        # Run install/activate in a subshell: cd'ing into emsdk/ here must not
        # leak into the rest of this script, or the GLM/stb clones below would
        # land inside emsdk/ instead of the project root.
        (
            cd emsdk
            ./emsdk install "${EMSDK_VERSION}"
            ./emsdk activate "${EMSDK_VERSION}"
        ) || {
            echo "error: emsdk install/activate failed."
            exit 1
        }
        # Dot (.), not exec: this must run IN the current shell so the PATH
        # and EMSDK env vars it sets survive after the script continues. A
        # plain ./emsdk/emsdk_env.sh would set them in a child process that
        # exits immediately, leaving this shell's PATH untouched.
        . ./emsdk/emsdk_env.sh
    fi
fi

command -v em++ >/dev/null 2>&1 || {
    echo "error: em++ still not found on PATH after the bootstrap attempt."
    echo "       Try activating emsdk manually: source ./emsdk/emsdk_env.sh"
    exit 1
}

mkdir -p web

# --- Fetch GLM (same tag the desktop build pulls via CMake FetchContent) -----
# GLM and stb are plain header-only git repos, not Emscripten ports, so
# Emscripten has no built-in mechanism to fetch them the way it does for SDL2
# below -- they need an explicit git clone here, same as the desktop build's
# FetchContent step.
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
#       Unlike GLM/stb above, this needs no git clone: it tells emcc to pull
#       a prebuilt-for-wasm SDL2 from Emscripten's own port system and cache
#       it inside your emsdk install, the first time you build.
#
#   -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
#       WebGL2 IS OpenGL ES 3.0. Replaces -sUSE_WEBGL2=1, which Emscripten now
#       marks deprecated. MIN=2 drops the WebGL1 fallback, which is useless to
#       us: there is no automatic downgrade, and ES 3.0 needs WebGL2 anyway.
#       Note there is no glad here at all: Emscripten supplies the GLES3
#       bindings itself, so the desktop build's glad/jinja2 dependency does
#       not apply to this target.
#
#   FULL_ES3 is deliberately NOT set. It emulates the GLES3 features WebGL2
#       lacks -- chiefly client-side vertex arrays -- and pulls in FULL_ES2,
#       costing code size and speed. main.cpp draws only from VBOs, so it is
#       not needed. Add it back if you hit an unsupported GLES3 path later.
#
#   --preload-file shaders / --preload-file images
#       Packs shaders/ and images/ into index.data, mounted in MEMFS.
#       main.cpp opens "../../shaders/...", and Emscripten's working
#       directory is "/", so that normalises to "/shaders/..." because paths
#       above the root get clamped. It works, but it is an accident of path
#       handling.
#
#   --shell-file shell.html
#       Custom HTML shell instead of Emscripten's default minimal page.
#
#   WASM=1 is the default now, so it is no longer passed explicitly.

# em++, NOT emcc. Emscripten links libc++/libc++abi only when LINK_AS_CXX is
# set, and that is driven by the driver name (tools/link.py: run_via_emxx).
# emcc compiles .cpp as C++ but does not link the C++ standard library, which
# shows up as undefined std::/__cxa_/operator new symbols at link time.
# (-sDEFAULT_TO_CXX=1 would also work; em++ is the documented way.)
em++ -std=c++20 -O2 \
    src/Application/main.cpp \
    src/Application/Application.cpp \
    src/Scene/GameScene.cpp \
    src/Input/InputManager.cpp \
    src/Renderer/Mesh.cpp \
    src/Renderer/ShaderHelper.cpp \
    src/Renderer/AssetManager.cpp \
    src/Renderer/Material.cpp \
    src/Renderer/stb_image_impl.cpp \
    -Isrc \
    -I"$GLM_DIR" \
    -I"$STB_DIR" \
    -sUSE_SDL=2 \
    -sMIN_WEBGL_VERSION=2 \
    -sMAX_WEBGL_VERSION=2 \
    -sALLOW_MEMORY_GROWTH=1 \
    --preload-file shaders \
    --preload-file images \
    --shell-file shell.html \
    -o web/index.html

# For a debug build, swap -O2 above for:  -O0 -g -gsource-map -sASSERTIONS=2

echo ""
echo "Build succeeded."
echo "Run:  emrun web/index.html"