# One-click build guide

`build-all.ps1` now attempts to install the complete build toolchain through `winget`:

- CMake
- Visual Studio 2022 C++ Build Tools
- Windows SDK component
- Official LunarG Vulkan SDK (`LunarG.VulkanSDK`)

The RDP/Windows machine needs internet access and `winget`/App Installer. Run by double-clicking:

```text
Maaz_DriversUltimatePRO\MaazXGPU\build-all.cmd
```

Or from the repository root:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\Maaz_DriversUltimatePRO\MaazXGPU\build-all.ps1 -ProjectRoot (Get-Location) -Clean -PackageZip
```

The script locates the newest installed SDK under `C:\VulkanSDK`, sets `VULKAN_SDK` for the current build, configures Visual Studio 2022 x64, builds Release, searches both `Build` and `Build\Release` for artifacts, and copies them to:

```text
%USERPROFILE%\Desktop\MaazXGPU-Driver\
%USERPROFILE%\Desktop\MaazXGPU-Driver.zip
```

If the Vulkan package is unavailable in your `winget` source, install LunarG Vulkan SDK manually and rerun the script. The script does not install `driver.inf` or replace the Intel display driver; MaazXGPU remains a user-mode performance layer.
