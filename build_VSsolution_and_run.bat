@echo off
REM =============================================================================
REM build_VSsolution_and_run.bat - Desktop build and run for GAM200
REM
REM Solution : GAM200  (build_desktop\GAM200.sln)
REM Projects : Engine  static library   (Engine\src)
REM            Game    executable       (Game\src)  -> build_desktop\Game\Debug\Game.exe
REM Toolchain: MSVC via the "Visual Studio 17 2022" generator, x64.
REM
REM Usage:
REM   build_VSsolution_and_run.bat                incremental configure + build + run
REM   build_VSsolution_and_run.bat CLEAN          wipe build_desktop first
REM   build_VSsolution_and_run.bat GET_VERSION    define GET_VERSION=1
REM   Flags combine, in any order:  build_VSsolution_and_run.bat CLEAN GET_VERSION
REM
REM Optional: pin the Python that glad's generator runs under.
REM   set GAM200_PYTHON=C:\Path\To\python.exe
REM =============================================================================

SETLOCAL

REM --- Project settings --------------------------------------------------------
REM The runnable target. Change this if you rename the Game project or set
REM OUTPUT_NAME in Game\CMakeLists.txt.
SET "APP_NAME=Game"
SET "BUILD_DIR=build_desktop"
SET "CONFIG=Debug"

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
    echo        CLEAN requested - removing %BUILD_DIR% ...
    IF EXIST %BUILD_DIR% RMDIR /S /Q %BUILD_DIR%
)
IF NOT EXIST %BUILD_DIR% MKDIR %BUILD_DIR%

SET "PY_FLAG="
IF DEFINED GAM200_PYTHON SET PY_FLAG=-DPython_EXECUTABLE="%GAM200_PYTHON:\=/%"

cmake -S . -B %BUILD_DIR% -G "Visual Studio 17 2022" -A x64 %PY_FLAG% %EXTRA_CMAKE_FLAGS%
IF %ERRORLEVEL% NEQ 0 ( echo Configuration failed. & pause & exit /b 1 )

echo        Solution: %BUILD_DIR%\GAM200.sln

echo [2/4] Checking glad's Python dependency ...

SET "CMAKE_PY="
FOR /F "tokens=2 delims==" %%P IN (
    'findstr /B /C:"_Python_EXECUTABLE:INTERNAL=" %BUILD_DIR%\CMakeCache.txt 2^>nul'
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
        echo            build_VSsolution_and_run.bat CLEAN
        echo.
        pause
        exit /b 1
    )
)

echo [3/4] Building %APP_NAME% ...
cmake --build %BUILD_DIR% --config %CONFIG% --target %APP_NAME%
IF %ERRORLEVEL% NEQ 0 ( echo Build failed. & pause & exit /b 1 )

echo [4/4] Running ...
echo.
echo Controls: ESC = quit
echo.

SET "EXE_DIR=%BUILD_DIR%\%APP_NAME%\%CONFIG%"
IF NOT EXIST "%EXE_DIR%\%APP_NAME%.exe" (
    REM Fall back to the shared output folder in case the project sets
    REM CMAKE_RUNTIME_OUTPUT_DIRECTORY.
    SET "EXE_DIR=%BUILD_DIR%\%CONFIG%"
)
IF NOT EXIST "%EXE_DIR%\%APP_NAME%.exe" (
    echo ERROR: %APP_NAME%.exe was not found under %BUILD_DIR%.
    echo        Looked in %BUILD_DIR%\%APP_NAME%\%CONFIG% and %BUILD_DIR%\%CONFIG%.
    pause
    exit /b 1
)

pushd "%EXE_DIR%"
%APP_NAME%.exe
popd

ENDLOCAL
pause