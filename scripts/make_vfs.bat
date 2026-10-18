@echo off
setlocal enabledelayedexpansion
set "ROOT=%~dp0.."
set "SAMPLES=%ROOT%\data\vfs_samples"
set "OUTPUT=%ROOT%\data\vfs"

if not exist "%OUTPUT%" mkdir "%OUTPUT%"

call :pack minimal
call :pack several
call :pack nested

if exist "%OUTPUT%\corrupt.zip" del "%OUTPUT%\corrupt.zip"
> "%OUTPUT%\corrupt.zip" echo this is not a zip archive
echo make_vfs: created %OUTPUT%\corrupt.zip
endlocal
exit /b 0

:pack
set "NAME=%~1"
if not exist "%SAMPLES%\%NAME%" (
    echo make_vfs: missing sample directory: %SAMPLES%\%NAME%
    exit /b 1
)
if exist "%OUTPUT%\%NAME%.zip" del "%OUTPUT%\%NAME%.zip"
powershell -NoProfile -Command ^
    "Compress-Archive -Path '%SAMPLES%\%NAME%\*' -DestinationPath '%OUTPUT%\%NAME%.zip' -Force"
if errorlevel 1 (
    echo make_vfs: failed to pack %NAME%
    exit /b 1
)
echo make_vfs: created %OUTPUT%\%NAME%.zip
exit /b 0
