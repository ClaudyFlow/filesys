@echo off
REM ============================================================
REM Build script for library-management-system
REM Requires: Ninja, CMake
REM ============================================================

SET "SRC_DIR=%~dp0"
SET "SRC_DIR=%SRC_DIR:~0,-1%"
SET "BUILD_DIR=%SRC_DIR%\build"
SET "BIN_DIR=%SRC_DIR%\bin"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"

cmake -G Ninja -B "%BUILD_DIR%" -S "%SRC_DIR%"
if errorlevel 1 (
    echo CMake failed.
    pause
    exit /b 1
)

cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)
