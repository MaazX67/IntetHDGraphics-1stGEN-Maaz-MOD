param(
    [string]$ProjectRoot = "",
    [switch]$SkipToolInstall,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

function Info([string]$Message) { Write-Host "[MaazXGPU] $Message" -ForegroundColor Cyan }
function Fail([string]$Message) { Write-Host "[MaazXGPU] ERROR: $Message" -ForegroundColor Red; exit 1 }

if (-not $ProjectRoot) {
    $ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
}
$ProjectRoot = (Resolve-Path $ProjectRoot).Path
$SourceDir = Join-Path $ProjectRoot "Maaz_DriversUltimatePRO\MaazXGPU"
$BuildDir = Join-Path $SourceDir "Build"
$OutputDir = Join-Path ([Environment]::GetFolderPath("Desktop")) "MaazXGPU-Driver"

if (-not (Test-Path (Join-Path $SourceDir "CMakeLists.txt"))) {
    Fail "CMakeLists.txt not found at $SourceDir. Pass -ProjectRoot with the repository root."
}

function HasCommand([string]$Name) {
    return $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

function Install-WingetPackage([string]$Id, [string]$Name, [string]$Override = "") {
    if (-not (HasCommand "winget")) {
        Fail "winget is unavailable. Install/update App Installer from Microsoft Store, then run this script again."
    }
    Info "Installing $Name ($Id)..."
    $args = @("install", "--id", $Id, "--exact", "--accept-source-agreements", "--accept-package-agreements", "--silent")
    if ($Override) { $args += @("--override", $Override) }
    & winget @args
    if ($LASTEXITCODE -ne 0) { Fail "Could not install $Name. Install it manually and rerun with -SkipToolInstall." }
}

if (-not $SkipToolInstall) {
    if (-not (HasCommand "cmake")) {
        Install-WingetPackage "Kitware.CMake" "CMake"
    }

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        $override = "--quiet --wait --norestart --nocache --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
        Install-WingetPackage "Microsoft.VisualStudio.2022.BuildTools" "Visual Studio 2022 Build Tools" $override
    }

    $vulkanRoot = $env:VULKAN_SDK
    if (-not $vulkanRoot -or -not (Test-Path (Join-Path $vulkanRoot "Include\vulkan\vulkan.h"))) {
        Info "VULKAN_SDK is not configured. Installing the LunarG Vulkan SDK package if available..."
        Install-WingetPackage "LunarG.VulkanSDK" "LunarG Vulkan SDK"
        $vulkanRoot = $env:VULKAN_SDK
    }
}

if (-not (HasCommand "cmake")) { Fail "CMake is not available in this shell. Open a new PowerShell window and rerun." }
if (-not $env:VULKAN_SDK) {
    $known = Get-ChildItem "C:\VulkanSDK" -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending | Select-Object -First 1
    if ($known) { $env:VULKAN_SDK = $known.FullName }
}
if (-not $env:VULKAN_SDK -or -not (Test-Path (Join-Path $env:VULKAN_SDK "Include\vulkan\vulkan.h"))) {
    Fail "Vulkan SDK was not found. Install the LunarG Vulkan SDK, set VULKAN_SDK, and rerun."
}

if ($Clean -and (Test-Path $BuildDir)) {
    Info "Removing previous build directory..."
    Remove-Item $BuildDir -Recurse -Force
}

Info "Configuring Visual Studio 2022 x64 build..."
& cmake -B $BuildDir -S $SourceDir -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { Fail "CMake configuration failed." }

Info "Compiling Release configuration..."
& cmake --build $BuildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { Fail "Compilation failed. Review the compiler error above." }

New-Item $OutputDir -ItemType Directory -Force | Out-Null
$releaseDir = Join-Path $BuildDir "Release"
$files = @("MaazXGPU.dll", "MaazXGPU.lib", "MaazXGPU_Tray.exe")
foreach ($file in $files) {
    $source = Join-Path $releaseDir $file
    if (Test-Path $source) { Copy-Item $source $OutputDir -Force }
}
Copy-Item (Join-Path $SourceDir "driver.inf") $OutputDir -Force
Copy-Item (Join-Path $SourceDir "README.txt") $OutputDir -Force

Info "Build completed. Files copied to: $OutputDir"
Get-ChildItem $OutputDir | Select-Object Name, Length, LastWriteTime
Write-Host ""
Write-Host "This is a user-mode compatibility/performance layer, not a signed kernel display driver." -ForegroundColor Yellow
