@echo off
REM =============================================================================
REM build_web.bat - Build GAM200 for WebGL2 (OpenGL ES 3.0) via Emscripten + SDL2
REM
REM Windows counterpart of build_web.sh. Keep the two in sync.
REM
REM Prerequisites:
REM   1. Install the Emscripten SDK: https://emscripten.org/docs/getting_started/
REM   2. This script activates .\emsdk automatically when em++ is not already on
REM      PATH. Otherwise run your emsdk's emsdk_env.bat first.
REM   3. Run from the project root:  build_web.bat
REM
REM Output: web\index.html  (+ index.js, index.wasm, index.data)
REM Serve:  cd web && python -m http.server 8080
REM Open:   http://localhost:8080/
REM
REM Backend: SDL2 only. The GLFW path is gone -- Platform.h and main.cpp are
REM SDL2-only now, so there is no second block to uncomment.
REM =============================================================================

SETLOCAL

REM --- Locate em++ ------------------------------------------------------------
REM em++, NOT emcc. Emscripten links libc++/libc++abi only when LINK_AS_CXX is
REM set, and that is driven by the driver name (tools\link.py: run_via_emxx).
REM emcc compiles .cpp as C++ but does not link the C++ standard library, which
REM shows up as undefined std::/__cxa_/operator new symbols at link time.
REM (-sDEFAULT_TO_CXX=1 would also work; em++ is the documented way.)

REM em++.exe, NOT bare em++. A Unix-style extensionless launcher (from a macOS
REM or Linux emsdk copied into a Windows tree) satisfies `WHERE em++` because
REM WHERE matches exact filenames before trying PATHEXT -- but cmd.exe cannot
REM execute it, so the guard passes and the compile dies with
REM "'em++' is not recognized". Only the .bat wrapper proves a Windows install.
WHERE em++.exe >nul 2>nul
IF ERRORLEVEL 1 (
    IF EXIST "%~dp0emsdk\emsdk_env.bat" (
        echo Activating emsdk from %~dp0emsdk ...
        CALL "%~dp0emsdk\emsdk_env.bat" >nul
    )
)

WHERE em++.exe >nul 2>nul
IF ERRORLEVEL 1 (
    echo error: em++ not found on PATH.
    echo        No em++.exe found. Either emsdk is not installed for Windows,
    echo        or emsdk\upstream holds a macOS/Linux toolchain - no .exe wrappers.
    echo        Check: dir emsdk\upstream\emscripten\em++.exe
    echo        Reinstall: cd emsdk ^&^& emsdk install latest ^&^& emsdk activate latest
    EXIT /B 1
)

IF NOT EXIST web MKDIR web

REM --- Fetch GLM (same tag the desktop build pulls via CMake FetchContent) -----
SET "GLM_DIR=third_party\glm"
SET "GLM_TAG=1.0.1"

IF NOT EXIST "%GLM_DIR%" (
    echo Fetching GLM %GLM_TAG% into %GLM_DIR% ...
    git clone --branch %GLM_TAG% --depth 1 https://github.com/g-truc/glm.git "%GLM_DIR%"
    IF ERRORLEVEL 1 (
        echo error: GLM fetch failed. Is git installed and on PATH?
        EXIT /B 1
    )
)

SET "STB_DIR=third_party\stb"

IF NOT EXIST "%STB_DIR%" (
    echo Fetching stb into %STB_DIR% ...
    git clone --depth 1 https://github.com/nothings/stb.git "%STB_DIR%"
    IF ERRORLEVEL 1 (
        echo error: stb fetch failed. Is git installed and on PATH?
        EXIT /B 1
    )
)

REM --- Build ------------------------------------------------------------------
REM Flag notes:
REM
REM   -sUSE_SDL=2
REM       SDL2 port. (--use-port=sdl2 is the newer spelling; both are current.)
REM
REM   -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
REM       WebGL2 IS OpenGL ES 3.0. Replaces -sUSE_WEBGL2=1, which Emscripten now
REM       marks deprecated. MIN=2 drops the WebGL1 fallback, which is useless to
REM       us: there is no automatic downgrade, and ES 3.0 needs WebGL2 anyway.
REM
REM   FULL_ES3 is deliberately NOT set. It emulates the GLES3 features WebGL2
REM       lacks -- chiefly client-side vertex arrays -- and pulls in FULL_ES2,
REM       costing code size and speed. main.cpp draws only from VBOs. Add it
REM       back if you hit an unsupported GLES3 path later.
REM
REM   --preload-file shaders
REM       Packs shaders\ into index.data, mounted at /shaders in MEMFS.
REM       main.cpp opens "../../shaders/...", and Emscripten's working directory
REM       is "/", so that normalises to "/shaders/..." because paths above the
REM       root get clamped. It works, but it is an accident of path handling.
REM
REM   WASM=1 is the default now, so it is no longer passed explicitly.

em++ -std=c++20 -O2 ^
    src/Application/main.cpp ^
    src/Application/Application.cpp ^
    src/Scene/GameScene.cpp ^
    src/Scene/Player.cpp ^
    src/Scene/Floor.cpp ^
    src/Input/InputManager.cpp ^
    src/Renderer/stb_image_impl.cpp ^
    src/Renderer/Mesh.cpp ^
    -Isrc ^
    -I"%GLM_DIR%" ^
    -I"%STB_DIR%" ^
    -sUSE_SDL=2 ^
    -sMIN_WEBGL_VERSION=2 ^
    -sMAX_WEBGL_VERSION=2 ^
    -sALLOW_MEMORY_GROWTH=1 ^
    --preload-file shaders ^
    --preload-file images ^
    --shell-file shell.html ^
    -o web/index.html

IF ERRORLEVEL 1 (
    echo.
    echo Build FAILED.
    EXIT /B 1
)

REM For a debug build, swap -O2 above for:  -O0 -g -gsource-map -sASSERTIONS=2

echo.
echo Build succeeded.
echo Run:  cd web ^&^& python -m http.server 8080
echo Open: http://localhost:8080/

ENDLOCAL
