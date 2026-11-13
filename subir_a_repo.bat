@echo off
setlocal

rem Run this script from any location; use the folder containing this file.
cd /d "%~dp0"
if errorlevel 1 goto error

where git >nul 2>&1
if errorlevel 1 (
    echo ERROR: Git is not installed or is not in PATH.
    goto error
)

set "REMOTE_URL=%~1"
if not defined REMOTE_URL set "REMOTE_URL=https://github.com/ingalfsan/BAS.git"

git rev-parse --is-inside-work-tree >nul 2>&1
if errorlevel 1 (
    git init -b main
    if errorlevel 1 goto error
)

git branch -M main
if errorlevel 1 goto error

git add -A
if errorlevel 1 goto error

echo.
echo Remote repository: %REMOTE_URL%
echo Files staged for upload:
git status --short
echo.
git diff --cached --stat
echo.
echo Review the staged files above and make sure no secrets or local-only files are included.
set "CONFIRM="
set /p "CONFIRM=Continue with commit and push? (Y/N): "
if /I not "%CONFIRM%"=="Y" (
    echo Cancelled. Staged files were left in the index; nothing was pushed.
    exit /b 1
)

git diff --cached --quiet
if errorlevel 1 (
    git commit -m "Initial BAS ESP32-S3 prototype"
    if errorlevel 1 goto error
) else (
    echo No new staged changes; using the existing commit.
)

git rev-parse --verify HEAD >nul 2>&1
if errorlevel 1 (
    echo ERROR: No commit exists to push.
    goto error
)

set "ORIGIN_URL="
for /f "delims=" %%R in ('git remote get-url origin 2^>nul') do set "ORIGIN_URL=%%R"
if defined ORIGIN_URL (
    if /I not "%ORIGIN_URL%"=="%REMOTE_URL%" (
        echo ERROR: origin already points to a different repository:
        echo %ORIGIN_URL%
        echo Update origin manually, then run this script again.
        goto error
    )
) else (
    git remote add origin "%REMOTE_URL%"
    if errorlevel 1 goto error
)

git push -u origin main
if errorlevel 1 (
    echo.
    echo Push failed. Check the URL, access permissions, and whether the remote is empty.
    echo No force-push was attempted.
    goto error
)

echo.
echo Upload complete.
exit /b 0

:error
echo.
echo Operation stopped. Review the message above; no history was rewritten.
exit /b 1
