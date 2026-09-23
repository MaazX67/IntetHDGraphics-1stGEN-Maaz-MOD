@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1" -ProjectRoot "%~dp0..\.." -Clean -PackageZip %*
if errorlevel 1 (
  echo.
  echo Build failed. Read the error above.
  pause
  exit /b 1
)
echo.
echo Build completed. Output is on the Desktop in MaazXGPU-Driver.
pause
