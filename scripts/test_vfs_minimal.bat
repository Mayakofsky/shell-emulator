@echo off
setlocal
set "ROOT=%~dp0.."
set "BUILD=%ROOT%\build"
set "VFS_OUT=%ROOT%\data\vfs"
if not exist "%BUILD%\shell-emulator.exe" (
    cmake -S "%ROOT%" -B "%BUILD%" -DCMAKE_BUILD_TYPE=Release
    if errorlevel 1 exit /b %errorlevel%
    cmake --build "%BUILD%" --config Release
    if errorlevel 1 exit /b %errorlevel%
)
call "%ROOT%\scripts\make_vfs.bat"
if errorlevel 1 exit /b %errorlevel%
"%BUILD%\shell-emulator.exe" --vfs "%VFS_OUT%\minimal.zip" --script "%ROOT%\scripts\startup_vfs.txt" <NUL
endlocal
