@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-all.ps1" %*
if errorlevel 1 (
  echo.
  echo MaazXGPU build failed. Read the error above.
  pause
  exit /b 1
)
echo.
echo MaazXGPU build completed successfully.
pause
