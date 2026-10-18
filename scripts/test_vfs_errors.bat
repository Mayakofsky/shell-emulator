@echo off
setlocal
set "ROOT=%~dp0.."
set "BUILD=%ROOT%\build"
set "VFS_OUT=%ROOT%\data\vfs"
set "LOG=%TEMP%\shell-vfs-errors.jsonl"
if not exist "%BUILD%\shell-emulator.exe" (
    cmake -S "%ROOT%" -B "%BUILD%" -DCMAKE_BUILD_TYPE=Release
    if errorlevel 1 exit /b %errorlevel%
    cmake --build "%BUILD%" --config Release
    if errorlevel 1 exit /b %errorlevel%
)
call "%ROOT%\scripts\make_vfs.bat"
if errorlevel 1 exit /b %errorlevel%
echo ---- nonexistent archive ----
"%BUILD%\shell-emulator.exe" --vfs "%VFS_OUT%\does-not-exist.zip" --script "%ROOT%\scripts\startup_vfs.txt" <NUL
echo exit code: %errorlevel%
echo ---- corrupt archive ----
"%BUILD%\shell-emulator.exe" --vfs "%VFS_OUT%\corrupt.zip" --script "%ROOT%\scripts\startup_vfs.txt" <NUL
echo exit code: %errorlevel%
echo ---- runtime vfs errors with logging ----
"%BUILD%\shell-emulator.exe" --vfs "%VFS_OUT%\several.zip" --log "%LOG%" ^
    --script "%ROOT%\scripts\startup_vfs_errors.txt" <NUL
type "%LOG%"
endlocal
