param(
    [string]$ProjectRoot = "",
    [switch]$SkipToolInstall,
    [switch]$Clean,
    [switch]$PackageZip
)
$ErrorActionPreference = "Stop"
function Info([string]$Message) { Write-Host "[MaazXGPU] $Message" -ForegroundColor Cyan }
function Fail([string]$Message) { Write-Host "[MaazXGPU] ERROR: $Message" -ForegroundColor Red; exit 1 }

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $ProjectRoot) { $ProjectRoot = Resolve-Path (Join-Path $scriptRoot "..\..\..") }
$ProjectRoot = (Resolve-Path $ProjectRoot).Path
$SourceDir = Join-Path $ProjectRoot "Maaz_DriversUltimatePRO\MaazXGPU"
if (Test-Path (Join-Path $ProjectRoot "CMakeLists.txt")) { $SourceDir = $ProjectRoot }
$BuildDir = Join-Path $SourceDir "Build"
$OutputDir = Join-Path ([Environment]::GetFolderPath("Desktop")) "MaazXGPU-Driver"
if (-not (Test-Path (Join-Path $SourceDir "CMakeLists.txt"))) { Fail "CMakeLists.txt not found. Pass -ProjectRoot with the repository root." }
function HasCommand([string]$Name) { return $null -ne (Get-Command $Name -ErrorAction SilentlyContinue) }
function Install-WingetPackage([string]$Id, [string]$Name, [string]$Override = "") {
    if (-not (HasCommand "winget")) { Fail "winget is unavailable. Install App Installer first." }
    Info "Installing $Name..."
    $args = @("install", "--id", $Id, "--exact", "--accept-source-agreements", "--accept-package-agreements", "--silent")
    if ($Override) { $args += @("--override", $Override) }
    & winget @args
    if ($LASTEXITCODE -ne 0) { Fail "Could not install $Name. Install it manually and rerun with -SkipToolInstall." }
}
if (-not $SkipToolInstall) {
    if (-not (HasCommand "cmake")) { Install-WingetPackage "Kitware.CMake" "CMake" }
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) { Install-WingetPackage "Microsoft.VisualStudio.2022.BuildTools" "Visual Studio 2022 Build Tools" "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended" }
}
if (-not (HasCommand "cmake")) { Fail "CMake is unavailable in this shell. Open a new terminal." }
if (-not $env:VULKAN_SDK) {
    $known = Get-ChildItem "C:\VulkanSDK" -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending | Select-Object -First 1
    if ($known) { $env:VULKAN_SDK = $known.FullName }
}
if (-not $env:VULKAN_SDK -or -not (Test-Path (Join-Path $env:VULKAN_SDK "Include\vulkan\vulkan.h"))) { Fail "Vulkan SDK not found. Install it and set VULKAN_SDK." }
if ($Clean -and (Test-Path $BuildDir)) { Remove-Item $BuildDir -Recurse -Force }
Info "Configuring x64 Release build..."
& cmake -B $BuildDir -S $SourceDir -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { Fail "CMake configuration failed." }
& cmake --build $BuildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { Fail "Compilation failed." }
New-Item $OutputDir -ItemType Directory -Force | Out-Null
$releaseDir = Join-Path $BuildDir "Release"
foreach ($file in @("MaazXGPU.dll", "MaazXGPU.lib", "MaazXGPU_Tray.exe")) {
    $source = Join-Path $releaseDir $file
    if (Test-Path $source) { Copy-Item $source $OutputDir -Force }
}
Copy-Item (Join-Path $SourceDir "driver.inf") $OutputDir -Force
Copy-Item (Join-Path $SourceDir "README.txt") $OutputDir -Force
if ($PackageZip) {
    $zip = Join-Path ([Environment]::GetFolderPath("Desktop")) "MaazXGPU-Driver.zip"
    if (Test-Path $zip) { Remove-Item $zip -Force }
    Compress-Archive -Path (Join-Path $OutputDir "*") -DestinationPath $zip
    Info "Package created: $zip"
}
Info "Build completed: $OutputDir"
Get-ChildItem $OutputDir | Select-Object Name, Length, LastWriteTime
Write-Host "This is a user-mode performance layer, not a signed kernel display driver." -ForegroundColor Yellow
