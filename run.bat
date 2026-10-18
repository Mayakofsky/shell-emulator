@echo off
setlocal
set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
set "BUILD_DIR=%ROOT%\build"

cmake -S "%ROOT%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b %errorlevel%

cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 exit /b %errorlevel%

if /I "%~1"=="test" goto test

set "EXECUTABLE=%BUILD_DIR%\shell-emulator.exe"
if exist "%BUILD_DIR%\Release\shell-emulator.exe" set "EXECUTABLE=%BUILD_DIR%\Release\shell-emulator.exe"
"%EXECUTABLE%"
exit /b %errorlevel%

:test
ctest --test-dir "%BUILD_DIR%" -C Release --output-on-failure
exit /b %errorlevel%
