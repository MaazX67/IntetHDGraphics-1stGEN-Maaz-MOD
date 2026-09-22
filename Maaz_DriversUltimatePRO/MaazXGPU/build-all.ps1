param(
    [string]$ProjectRoot = "",
    [switch]$SkipToolInstall,
    [switch]$Clean,
    [switch]$PackageZip,
    [switch]$NoPause
)
$ErrorActionPreference = "Stop"

function Info([string]$Message) { Write-Host "[MaazXGPU] $Message" -ForegroundColor Cyan }
function Warn([string]$Message) { Write-Host "[MaazXGPU] WARNING: $Message" -ForegroundColor Yellow }
function Fail([string]$Message) { Write-Host "[MaazXGPU] ERROR: $Message" -ForegroundColor Red; exit 1 }
function HasCommand([string]$Name) { return $null -ne (Get-Command $Name -ErrorAction SilentlyContinue) }

function Install-WingetPackage([string]$Id, [string]$Name, [string]$Override = "") {
    if (-not (HasCommand "winget")) {
        Fail "winget is unavailable. Install/update App Installer from Microsoft Store, then run this script again."
    }
    Info "Installing or updating $Name..."
    $arguments = @("install", "--id", $Id, "--exact", "--source", "winget",
                   "--accept-source-agreements", "--accept-package-agreements", "--silent")
    if ($Override) { $arguments += @("--override", $Override) }
    & winget @arguments
    if ($LASTEXITCODE -ne 0) {
        Fail "$Name installation failed. Install it manually, then rerun with -SkipToolInstall."
    }
}

# Resolve the repository root regardless of whether this script is launched from
# the source directory, repository root, or Desktop.
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $ProjectRoot) {
    $candidate = Resolve-Path (Join-Path $scriptRoot "..\..") -ErrorAction SilentlyContinue
    if ($candidate) { $ProjectRoot = $candidate.Path } else { $ProjectRoot = $scriptRoot }
}
$ProjectRoot = (Resolve-Path $ProjectRoot).Path
$SourceDir = Join-Path $ProjectRoot "Maaz_DriversUltimatePRO\MaazXGPU"
if (Test-Path (Join-Path $ProjectRoot "CMakeLists.txt")) { $SourceDir = $ProjectRoot }
$BuildDir = Join-Path $SourceDir "Build"
$Desktop = [Environment]::GetFolderPath("Desktop")
$OutputDir = Join-Path $Desktop "MaazXGPU-Driver"

if (-not (Test-Path (Join-Path $SourceDir "CMakeLists.txt"))) {
    Fail "CMakeLists.txt not found. Use -ProjectRoot with the repository root."
}

if (-not $SkipToolInstall) {
    if (-not (HasCommand "winget")) {
        Fail "winget is required for automatic installation. Install App Installer first."
    }
    if (-not (HasCommand "cmake")) {
        Install-WingetPackage "Kitware.CMake" "CMake"
    }
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        $override = "--quiet --wait --norestart --nocache --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
        Install-WingetPackage "Microsoft.VisualStudio.2022.BuildTools" "Visual Studio 2022 C++ Build Tools" $override
    }
}

if (-not (HasCommand "cmake")) {
    Fail "CMake is not available. Open a new PowerShell window after installing it."
}

# The user-mode MaazXGPU build needs Vulkan headers/libraries. Do not silently
# pretend the WDK is enough; WDK is only for the separate experimental kernel folder.
if (-not $env:VULKAN_SDK) {
    $known = Get-ChildItem "C:\VulkanSDK" -Directory -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending | Select-Object -First 1
    if ($known) { $env:VULKAN_SDK = $known.FullName }
}
if (-not $env:VULKAN_SDK -or -not (Test-Path (Join-Path $env:VULKAN_SDK "Include\vulkan\vulkan.h"))) {
    Warn "VULKAN_SDK was not found. Install the LunarG Vulkan SDK, restart the terminal, and run again."
    Warn "Automatic Vulkan installation is intentionally not forced because package IDs and SDK versions vary."
    Fail "Vulkan SDK is required for MaazXGPU."
}

if ($Clean -and (Test-Path $BuildDir)) {
    Info "Removing old build directory..."
    Remove-Item $BuildDir -Recurse -Force
}

Info "Configuring MaazXGPU with Visual Studio 2022 x64..."
& cmake -B $BuildDir -S $SourceDir -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { Fail "CMake configuration failed." }

Info "Building Release configuration..."
& cmake --build $BuildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { Fail "Compilation failed. The compiler output above contains the cause." }

New-Item $OutputDir -ItemType Directory -Force | Out-Null
$releaseDir = Join-Path $BuildDir "Release"
foreach ($file in @("MaazXGPU.dll", "MaazXGPU.lib", "MaazXGPU_Tray.exe")) {
    $source = Join-Path $releaseDir $file
    if (Test-Path $source) { Copy-Item $source $OutputDir -Force }
}
foreach ($file in @("driver.inf", "README.txt", "BUILD-INSTRUCTIONS.md")) {
    $source = Join-Path $SourceDir $file
    if (Test-Path $source) { Copy-Item $source $OutputDir -Force }
}

if ($PackageZip) {
    $zip = Join-Path $Desktop "MaazXGPU-Driver.zip"
    if (Test-Path $zip) { Remove-Item $zip -Force }
    Compress-Archive -Path (Join-Path $OutputDir "*") -DestinationPath $zip -Force
    Info "Package created: $zip"
}

Info "Build completed. Output: $OutputDir"
Get-ChildItem $OutputDir | Select-Object Name, Length, LastWriteTime
Write-Host ""
Warn "This output is a user-mode performance layer, not a signed kernel display driver."
Warn "Keep the existing Intel graphics driver installed; do not install driver.inf as a replacement."

if (-not $NoPause) { Read-Host "Press Enter to close" | Out-Null }
