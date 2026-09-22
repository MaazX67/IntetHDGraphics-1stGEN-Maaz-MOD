@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-driver.ps1" %*
if errorlevel 1 (
  echo.
  echo Build failed. Keep this window open to read the error.
  pause
  exit /b 1
)
echo.
echo Build completed successfully.
pause
