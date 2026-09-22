# MaazXGPU build helper

Copy `build-driver.ps1` or `build-driver.cmd` to the repository root or Desktop. The script expects this layout:

```text
<project root>\Maaz_DriversUltimatePRO\MaazXGPU\CMakeLists.txt
```

From PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\build-driver.ps1 -ProjectRoot "C:\Path\To\Repository" -Clean
```

From Command Prompt:

```cmd
build-driver.cmd -ProjectRoot "C:\Path\To\Repository" -Clean
```

The script attempts to install these dependencies through `winget`:

- CMake (`Kitware.CMake`)
- Visual Studio 2022 Build Tools with the C++ workload
- LunarG Vulkan SDK (`LunarG.VulkanSDK`)

After compilation, it copies available artifacts to:

```text
%USERPROFILE%\Desktop\MaazXGPU-Driver\
```

The script does not install `driver.inf` and does not replace the Intel display driver. This project is a user-mode compatibility/performance layer, not a signed kernel display driver.
