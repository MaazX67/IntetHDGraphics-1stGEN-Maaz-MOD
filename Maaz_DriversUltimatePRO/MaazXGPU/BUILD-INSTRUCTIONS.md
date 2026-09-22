# One-click build guide

## Main MaazXGPU build

This project is a user-mode performance layer. It is built beside the installed Intel graphics driver; it does not replace the Intel WDDM driver.

1. Put the repository anywhere on the Windows PC or RDP desktop.
2. Install/update App Installer so `winget` works.
3. Double-click:

```text
Maaz_DriversUltimatePRO\MaazXGPU\build-all.cmd
```

The script automatically installs/checks:

- CMake
- Visual Studio 2022 C++ Build Tools

The Vulkan SDK must be installed separately because Vulkan package IDs and SDK versions can change. Download and install the LunarG Vulkan SDK, then open a new terminal. Verify:

```powershell
$env:VULKAN_SDK
Test-Path "$env:VULKAN_SDK\Include\vulkan\vulkan.h"
```

Or run explicitly from the repository root:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\Maaz_DriversUltimatePRO\MaazXGPU\build-all.ps1 -ProjectRoot (Get-Location) -Clean -PackageZip
```

Output:

```text
%USERPROFILE%\Desktop\MaazXGPU-Driver\
%USERPROFILE%\Desktop\MaazXGPU-Driver.zip
```

## How it works with Intel

```text
Minecraft / Roblox / DX12 game
          ↓
MaazXGPU_Tray.exe + MaazXGPU.dll
          ↓
Existing Intel graphics driver / WDDM
          ↓
Intel HD Graphics hardware
```

The tray applies reversible user-mode game profiles: process detection, high priority, throttling opt-out when Windows permits it, shader/cache support, and conservative low-memory handling. It does not create dedicated VRAM, guarantee 30 FPS, or replace the Intel driver.

Start the tray manually from the Desktop output directory:

```powershell
Start-Process "$env:USERPROFILE\Desktop\MaazXGPU-Driver\MaazXGPU_Tray.exe"
```

The project also contains `experimental-wddm-driver`, but that folder is a separate inert WDK research skeleton. It has no hardware IDs or installable INF and must not be used to replace Intel on the main machine.
