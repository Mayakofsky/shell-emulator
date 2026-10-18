@echo off
setlocal
set "ROOT=%~dp0.."
set "BUILD=%ROOT%\build"
cd /d "%ROOT%"
cmake -S "%ROOT%" -B "%BUILD%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b %errorlevel%
cmake --build "%BUILD%" --config Release
if errorlevel 1 exit /b %errorlevel%
call "%ROOT%\scripts\make_vfs.bat"
if errorlevel 1 exit /b %errorlevel%
"%BUILD%\shell-emulator.exe" --config "%ROOT%\scripts\config.xml" <NUL
type "%ROOT%\data\config-log.jsonl"
endlocal
