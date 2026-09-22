@echo off
cd /d "%~dp0"
echo === MaazXGPU Fast Build - No VS ===
echo.

REM Check MinGW
where gcc >nul 2>nul
if %errorlevel% neq 0 (
  echo MinGW nahi hai, install kar raha hu...
  winget install Mingw.Mingw --silent --accept-package-agreements
  set PATH=%PATH%;C:\ProgramData\mingw64\mingw64\bin
)

echo [1/3] Cleaning old Build...
rmdir /s /q Build 2>nul

echo [2/3] Generating with MinGW...
cmake -B Build -S . -G "MinGW Makefiles"
if %errorlevel% neq 0 (
  echo CMake fail, Zig se try kar raha hu...
  where zig >nul 2>nul
  if %errorlevel% neq 0 winget install zig.zig --silent
  zig c++ -shared src/*.cpp -I src -o MaazXGPU.dll -O2 -D MAAZXGPU_EXPORTS
  goto :done
)

echo [3/3] Compiling...
cmake --build Build --config Release

:done
echo.
echo === BUILD DONE ===
dir Build\*.dll 2>nul
dir *.dll 2>nul
dir Build\Release\*.dll 2>nul
echo.
echo Agar MaazXGPU.dll dikh raha hai to driver ban gaya!
pause