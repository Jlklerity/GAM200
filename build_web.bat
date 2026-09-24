@echo off
REM =============================================================================
REM build_web.bat - Build GAM200 for WebGL2 (OpenGL ES 3.0) via Emscripten + SDL2
REM
REM Windows counterpart of build_web.sh. Keep the two in sync.
REM
REM Prerequisites:
REM   None you need to install by hand. If em++.exe is not already on PATH,
REM   this script will bootstrap emsdk into .\emsdk itself (see below). If
REM   you'd rather manage emsdk yourself, run its emsdk_env.bat first.
REM
REM Run from the project root:  build_web.bat
REM
REM Output: web\index.html  (+ index.js, index.wasm, index.data)
REM Run:    emrun web\index.html
REM         (emrun ships with emsdk and is on PATH once emsdk_env.bat has run
REM         -- either by you, or by the bootstrap step below. It starts a
REM         local server, opens your default browser, and forwards the
REM         page's console output back into this window, which
REM         python -m http.server never did.)
REM
REM Backend: SDL2 only. The GLFW path is gone -- Platform.h and main.cpp are
REM SDL2-only now, so there is no second block to uncomment.
REM =============================================================================

SETLOCAL

REM --- Locate or bootstrap em++ ------------------------------------------------
REM emsdk is not a normal library dependency: it is an entire toolchain (its
REM own clang, its own linker, its own copy of Python for emrun/glad-style
REM helper scripts), so "just fetch a git repo" is not enough on its own -- it
REM also has to be installed (fetches the actual prebuilt binaries for this
REM OS/arch) and activated (writes .emscripten config + PATH additions).
REM
REM EMSDK_VERSION is pinned, not "latest", so a build from a fresh checkout
REM today reproduces the same toolchain a build six months from now would
REM use. Bump it deliberately, in its own commit, when you want to move --
REM keep this in sync with the version the README tells desktop contributors
REM to install, and with build_web.sh's EMSDK_VERSION.
SET "EMSDK_VERSION=6.0.9"

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
        echo Activating existing emsdk from %~dp0emsdk ...
        CALL "%~dp0emsdk\emsdk_env.bat" >nul
    ) ELSE (
        echo emsdk not found -- bootstrapping emsdk %EMSDK_VERSION% into %~dp0emsdk ...
        git clone https://github.com/emscripten-core/emsdk.git "%~dp0emsdk"
        IF ERRORLEVEL 1 (
            echo error: emsdk clone failed. Is git installed and on PATH?
            EXIT /B 1
        )

        REM PUSHD/POPD, not CD: like the subshell in build_web.sh, the
        REM directory change to run emsdk's own install/activate must not
        REM leak into the rest of this script, or the GLM/stb clones below
        REM would land inside emsdk\ instead of the project root.
        PUSHD "%~dp0emsdk"

        REM CALL is required here: emsdk.bat is itself a batch script, and
        REM without CALL, control would jump into it and never return to
        REM this script (the classic "batch calling batch" trap).
        CALL emsdk install %EMSDK_VERSION%
        IF ERRORLEVEL 1 (
            POPD
            echo error: emsdk install failed.
            EXIT /B 1
        )
        CALL emsdk activate %EMSDK_VERSION%
        IF ERRORLEVEL 1 (
            POPD
            echo error: emsdk activate failed.
            EXIT /B 1
        )
        POPD

        CALL "%~dp0emsdk\emsdk_env.bat" >nul
    )
)

WHERE em++.exe >nul 2>nul
IF ERRORLEVEL 1 (
    echo error: em++ not found on PATH after bootstrap attempt.
    echo        No em++.exe found. Either emsdk is not installed for Windows,
    echo        or emsdk\upstream holds a macOS/Linux toolchain - no .exe wrappers.
    echo        Check: dir emsdk\upstream\emscripten\em++.exe
    echo        Reinstall: cd emsdk ^&^& emsdk install %EMSDK_VERSION% ^&^& emsdk activate %EMSDK_VERSION%
    EXIT /B 1
)

IF NOT EXIST web MKDIR web

REM --- Fetch GLM (same tag the desktop build pulls via CMake FetchContent) -----
REM GLM and stb are plain header-only git repos, not Emscripten ports, so
REM Emscripten has no built-in mechanism to fetch them the way it does for
REM SDL2 below -- they need an explicit git clone here, same as the desktop
REM build's FetchContent step.
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
REM       Unlike GLM/stb above, this needs no git clone: it tells emcc to pull
REM       a prebuilt-for-wasm SDL2 from Emscripten's own port system and
REM       cache it inside your emsdk install, the first time you build.
REM
REM   -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
REM       WebGL2 IS OpenGL ES 3.0. Replaces -sUSE_WEBGL2=1, which Emscripten now
REM       marks deprecated. MIN=2 drops the WebGL1 fallback, which is useless to
REM       us: there is no automatic downgrade, and ES 3.0 needs WebGL2 anyway.
REM       Note there is no glad here at all: Emscripten supplies the GLES3
REM       bindings itself, so the desktop build's glad/jinja2 dependency does
REM       not apply to this target.
REM
REM   FULL_ES3 is deliberately NOT set. It emulates the GLES3 features WebGL2
REM       lacks -- chiefly client-side vertex arrays -- and pulls in FULL_ES2,
REM       costing code size and speed. main.cpp draws only from VBOs. Add it
REM       back if you hit an unsupported GLES3 path later.
REM
REM   --preload-file shaders / --preload-file images
REM       Packs shaders\ and images\ into index.data, mounted at /shaders and
REM       /images in MEMFS. main.cpp opens "../../shaders/...", and
REM       Emscripten's working directory is "/", so that normalises to
REM       "/shaders/..." because paths above the root get clamped. It works,
REM       but it is an accident of path handling.
REM
REM   --shell-file shell.html
REM       Custom HTML shell instead of Emscripten's default minimal page.
REM
REM   WASM=1 is the default now, so it is no longer passed explicitly.

em++ -std=c++20 -O2 ^
    src/Application/main.cpp ^
    src/Application/Application.cpp ^
    src/Scene/GameScene.cpp ^
    src/Input/InputManager.cpp ^
    src/Renderer/Mesh.cpp ^
    src/Renderer/ShaderHelper.cpp ^
    src/Renderer/AssetManager.cpp ^
    src/Renderer/Material.cpp ^
    src/Renderer/stb_image_impl.cpp ^
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

echo ""
echo "Build succeeded."
echo "Launching: emrun web/index.html"
emrun web/index.html || {
    echo "error: emrun failed to launch."
    echo "       Activate emsdk yourself and retry:"
    echo "       . ./emsdk/emsdk_env.sh && emrun web/index.html"
    exit 1
}

ENDLOCAL