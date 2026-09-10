@echo off
REM =============================================================================
REM script_build_and_run.bat - Desktop build and run for GAM200
REM
REM Toolchain: MSVC via the "Visual Studio 17 2022" generator, x64.
REM
REM Usage:
REM   script_build_and_run.bat                incremental configure + build + run
REM   script_build_and_run.bat CLEAN          wipe build_desktop first
REM   script_build_and_run.bat GET_VERSION    define GET_VERSION=1
REM   Flags combine, in any order:  script_build_and_run.bat CLEAN GET_VERSION
REM
REM Optional: pin the Python that glad's generator runs under.
REM   set GAM200_PYTHON=C:\Path\To\python.exe
REM =============================================================================

SETLOCAL

REM --- Parse flags -------------------------------------------------------------
SET "EXTRA_CMAKE_FLAGS="
SET "DO_CLEAN="

:parse
IF "%~1"=="" GOTO parsed
IF /I "%~1"=="GET_VERSION" (
    echo [INFO] GET_VERSION flag detected. Enabling OpenGL version info...
    SET EXTRA_CMAKE_FLAGS=-DCMAKE_CXX_FLAGS="/DGET_VERSION=1"
)
IF /I "%~1"=="CLEAN" (
    SET "DO_CLEAN=1"
)
SHIFT
GOTO parse
:parsed

cmake --version >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found in PATH.
    pause
    exit /b 1
)

REM --- [1/4] Configure ---------------------------------------------------------
REM RMDIR of build_desktop also destroys _deps\, which re-clones SDL2 and rebuilds
REM it from scratch on every run. CMake reconfigures incrementally, so the wipe is
REM opt-in via CLEAN.
echo [1/4] Configuring ...

IF DEFINED DO_CLEAN (
    echo        CLEAN requested - removing build_desktop ...
    IF EXIST build_desktop RMDIR /S /Q build_desktop
)
IF NOT EXIST build_desktop MKDIR build_desktop

SET "PY_FLAG="
IF DEFINED GAM200_PYTHON SET PY_FLAG=-DPython_EXECUTABLE="%GAM200_PYTHON:\=/%"

cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64 %PY_FLAG% %EXTRA_CMAKE_FLAGS%
IF %ERRORLEVEL% NEQ 0 ( echo Configuration failed. & pause & exit /b 1 )

REM --- [2/4] glad code-generator preflight -------------------------------------
REM glad2 ships no generated sources. glad_add_library() emits a custom build step
REM that runs `python -m glad`, and glad imports jinja2 to render its C templates
REM (_deps\glad-src\requirements.txt: Jinja2>=2.7,<4.0). A missing jinja2 surfaces
REM as MSB8066 on glad_gles2.vcxproj, which names MSBuild rather than the cause.
REM find_package(Python) chooses the interpreter, so read the one CMake actually
REM recorded instead of guessing from PATH - they are frequently different.
echo [2/4] Checking glad's Python dependency ...

SET "CMAKE_PY="
FOR /F "tokens=2 delims==" %%P IN (
    'findstr /B /C:"_Python_EXECUTABLE:INTERNAL=" build_desktop\CMakeCache.txt 2^>nul'
) DO SET "CMAKE_PY=%%P"
SET "CMAKE_PY=%CMAKE_PY:/=\%"

IF NOT DEFINED CMAKE_PY (
    echo        WARNING: could not read Python_EXECUTABLE from CMakeCache.txt.
    echo        Skipping the check; glad may still fail during the build.
) ELSE (
    echo        Interpreter: %CMAKE_PY%
    "%CMAKE_PY%" -c "import jinja2" >nul 2>nul
    IF ERRORLEVEL 1 (
        echo.
        echo ERROR: Python module 'jinja2' is missing from the interpreter CMake chose:
        echo            %CMAKE_PY%
        echo        glad cannot generate the OpenGL ES 3.0 loader without it.
        echo.
        echo        Fix:
        echo            "%CMAKE_PY%" -m pip install jinja2
        echo.
        echo        Then rerun this script. To use a different Python instead:
        echo            set GAM200_PYTHON=C:\Path\To\python.exe
        echo            script_build_and_run.bat CLEAN
        echo.
        pause
        exit /b 1
    )
)

REM --- [3/4] Build -------------------------------------------------------------
echo [3/4] Building ...
cmake --build build_desktop --config Debug
IF %ERRORLEVEL% NEQ 0 ( echo Build failed. & pause & exit /b 1 )

REM --- [4/4] Run ---------------------------------------------------------------
echo [4/4] Running ...
echo.
echo Controls: ESC = quit
echo.
pushd build_desktop\Debug
GAM200.exe
popd

ENDLOCAL
pause
