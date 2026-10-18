@echo off
setlocal
set "ROOT=%~dp0.."
set "BUILD=%ROOT%\build"
set "LOG=%TEMP%\shell-all.jsonl"
cd /d "%ROOT%"
cmake -S "%ROOT%" -B "%BUILD%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b %errorlevel%
cmake --build "%BUILD%" --config Release
if errorlevel 1 exit /b %errorlevel%
"%BUILD%\shell-emulator.exe" --config "%ROOT%\scripts\config.xml" --vfs cli-vfs ^
    --log "%LOG%" --script "%ROOT%\scripts\startup_demo.txt" <NUL
type "%LOG%"
endlocal
