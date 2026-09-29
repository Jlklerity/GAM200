@echo off
REM =============================================================================
REM generate_VSsolution.bat - Generate build_desktop\GAM200.sln and open it.
REM
REM Configure only: nothing in Engine or Game is compiled, so this works even
REM while the sources do not build. Use it to get a solution to edit in.
REM
REM Usage:
REM   generate_VSsolution.bat           configure + open the solution
REM   generate_VSsolution.bat CLEAN     wipe build_desktop first
REM
REM Optional: pin the Python that glad's generator runs under.
REM   set GAM200_PYTHON=C:\Path\To\python.exe
REM =============================================================================

SETLOCAL

SET "BUILD_DIR=build_desktop"
SET "DO_CLEAN="

:parse
IF "%~1"=="" GOTO parsed
IF /I "%~1"=="CLEAN" SET "DO_CLEAN=1"
SHIFT
GOTO parse
:parsed

cmake --version >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found in PATH.
    pause
    exit /b 1
)

IF DEFINED DO_CLEAN (
    echo CLEAN requested - removing %BUILD_DIR% ...
    IF EXIST %BUILD_DIR% RMDIR /S /Q %BUILD_DIR%
)
IF NOT EXIST %BUILD_DIR% MKDIR %BUILD_DIR%

SET "PY_FLAG="
IF DEFINED GAM200_PYTHON SET PY_FLAG=-DPython_EXECUTABLE="%GAM200_PYTHON:\=/%"

echo [1/3] Configuring ...
cmake -S . -B %BUILD_DIR% -G "Visual Studio 17 2022" -A x64 %PY_FLAG%
IF %ERRORLEVEL% NEQ 0 ( echo Configuration failed. & pause & exit /b 1 )

REM glad's header (glad/gles2.h) is generated at build time, not configure time.
REM Building only that target makes IntelliSense resolve it, and does not touch
REM Engine or Game. A failure here (e.g. missing jinja2) is not fatal.
echo [2/3] Generating glad loader for IntelliSense ...
cmake --build %BUILD_DIR% --config Debug --target glad_gles2
IF %ERRORLEVEL% NEQ 0 (
    echo WARNING: glad generation failed. The solution will still open, but
    echo          glad/gles2.h may show as missing until glad builds.
)

echo [3/3] Opening %BUILD_DIR%\GAM200.sln ...
start "" "%BUILD_DIR%\GAM200.sln"

ENDLOCAL