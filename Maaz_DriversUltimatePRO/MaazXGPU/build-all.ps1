param(
    [string]$ProjectRoot = "",
    [switch]$SkipToolInstall,
    [switch]$SkipVulkanInstall,
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
    if (-not (HasCommand "winget")) { Fail "winget is unavailable. Install App Installer from Microsoft Store first." }
    Info "Installing or updating $Name..."
    $arguments = @("install", "--id", $Id, "--exact", "--source", "winget",
        "--accept-source-agreements", "--accept-package-agreements", "--silent")
    if ($Override) { $arguments += @("--override", $Override) }
    & winget @arguments
    if ($LASTEXITCODE -ne 0) { Fail "$Name installation failed. Install it manually or rerun with the appropriate Skip switch." }
}

function Find-VulkanSdk {
    if ($env:VULKAN_SDK -and (Test-Path (Join-Path $env:VULKAN_SDK "Include\vulkan\vulkan.h"))) {
        return (Resolve-Path $env:VULKAN_SDK).Path
    }
    $roots = @("C:\VulkanSDK", "$env:ProgramFiles\VulkanSDK", "$env:ProgramFiles(x86)\VulkanSDK")
    foreach ($root in $roots) {
        if (Test-Path $root) {
            $candidate = Get-ChildItem $root -Directory -ErrorAction SilentlyContinue |
                Sort-Object Name -Descending | Where-Object {
                    Test-Path (Join-Path $_.FullName "Include\vulkan\vulkan.h")
                } | Select-Object -First 1
            if ($candidate) { return $candidate.FullName }
        }
    }
    return $null
}

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $ProjectRoot) {
    $candidate = Resolve-Path (Join-Path $scriptRoot "..\..") -ErrorAction SilentlyContinue
    $ProjectRoot = if ($candidate) { $candidate.Path } else { $scriptRoot }
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
    if (-not (HasCommand "winget")) { Fail "winget is required for automatic installation." }
    if (-not (HasCommand "cmake")) { Install-WingetPackage "Kitware.CMake" "CMake" }
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        $override = "--quiet --wait --norestart --nocache --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.Windows10SDK.19041 --includeRecommended"
        Install-WingetPackage "Microsoft.VisualStudio.2022.BuildTools" "Visual Studio 2022 C++ Build Tools and Windows SDK" $override
    }
}
if (-not (HasCommand "cmake")) { Fail "CMake is unavailable. Open a new terminal after installation." }

$vulkan = Find-VulkanSdk
if (-not $vulkan -and -not $SkipVulkanInstall) {
    Info "Vulkan SDK not found. Installing the official LunarG Vulkan SDK through winget..."
    Install-WingetPackage "LunarG.VulkanSDK" "LunarG Vulkan SDK"
    # winget installers commonly update the machine environment only for new shells.
    $vulkan = Find-VulkanSdk
}
if (-not $vulkan) {
    Fail "Vulkan SDK not found. Install LunarG Vulkan SDK, open a new terminal, or use -SkipVulkanInstall only for a non-Vulkan test." 
}
$env:VULKAN_SDK = $vulkan
Info "Using Vulkan SDK: $env:VULKAN_SDK"

if ($Clean -and (Test-Path $BuildDir)) { Info "Removing old build directory..."; Remove-Item $BuildDir -Recurse -Force }
Info "Configuring Visual Studio 2022 x64..."
& cmake -B $BuildDir -S $SourceDir -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { Fail "CMake configuration failed." }
Info "Building Release configuration..."
& cmake --build $BuildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { Fail "Compilation failed. Review the compiler output above." }

New-Item $OutputDir -ItemType Directory -Force | Out-Null
# CMake may place multi-config outputs in Build or Build/Release depending on target properties.
$artifacts = Get-ChildItem $BuildDir -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -in @("MaazXGPU.dll", "MaazXGPU.lib", "MaazXGPU_Tray.exe") }
foreach ($artifact in $artifacts) { Copy-Item $artifact.FullName (Join-Path $OutputDir $artifact.Name) -Force }
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
Warn "This is a user-mode performance layer, not a signed kernel display driver. Keep the Intel driver installed."
if (-not $NoPause) { Read-Host "Press Enter to close" | Out-Null }
