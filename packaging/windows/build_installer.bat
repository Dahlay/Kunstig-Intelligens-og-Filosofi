@echo off
REM packaging/windows/build_installer.bat
REM
REM Run this on a Windows build machine to produce the NSIS installer.
REM
REM Prerequisites:
REM   - CMake in PATH
REM   - NSIS (makensis) in PATH
REM   - Visual Studio 2022 or Build Tools (for MSVC + Windows SDK)
REM   - A vcpkg or FetchContent internet connection for first build

setlocal enabledelayedexpansion

set BUILD_DIR=build_win
set CONFIG=Release

echo === Configuring CMake ===
cmake -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_BUILD_TYPE=%CONFIG% ^
      -DBUILD_TESTING=OFF
if errorlevel 1 ( echo CMake configure failed & exit /b 1 )

echo === Building ===
cmake --build "%BUILD_DIR%" --config %CONFIG% -j
if errorlevel 1 ( echo Build failed & exit /b 1 )

echo === Packaging with NSIS ===
cd packaging\windows
makensis installer.nsi
if errorlevel 1 ( echo NSIS packaging failed & exit /b 1 )
cd ..\..

echo === Done ===
echo Installer: packaging\windows\ModularAudioPatcher-1.0.0-Setup.exe
