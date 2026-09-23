# Fast local build

Use only `build.cmd` or `build.ps1`. The script does **not** download the 3GB Visual Studio IDE or install Vulkan automatically.

## Required once

Install these small/required components manually:

1. **CMake**
2. **Visual Studio Build Tools** with:
   - MSVC C++ x64/x86 build tools
   - Windows 10/11 SDK
3. **LunarG Vulkan SDK**

After installing Build Tools, open **Developer PowerShell for VS**. Confirm:

```powershell
cmake --version
cl
Test-Path "$env:VULKAN_SDK\Include\vulkan\vulkan.h"
Test-Path "$env:VULKAN_SDK\Lib\vulkan-1.lib"
```

The last two commands must return `True`.

## Compile

Double-click:

```text
Maaz_DriversUltimatePRO\MaazXGPU\build.cmd
```

Or from Developer PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\Maaz_DriversUltimatePRO\MaazXGPU\build.ps1 -Clean -PackageZip
```

Output:

```text
%USERPROFILE%\Desktop\MaazXGPU-Driver\
%USERPROFILE%\Desktop\MaazXGPU-Driver.zip
```

This builds the user-mode MaazXGPU layer and tray app. Keep the Intel graphics driver installed; do not install `driver.inf` as a replacement display driver.

If `CMakeLists.txt`, Vulkan, or `cl.exe` is missing, the script stops with a clear message instead of downloading a large package.
