param(
    [string]$ProjectRoot = "",
    [switch]$Clean,
    [switch]$PackageZip,
    [switch]$InstallMissingTools,
    [switch]$NoPause
)
$ErrorActionPreference = "Stop"

function Info($m) { Write-Host "[MaazXGPU] $m" -ForegroundColor Cyan }
function Fail($m) { Write-Host "[MaazXGPU] ERROR: $m" -ForegroundColor Red; exit 1 }
function Has($name) { return $null -ne (Get-Command $name -ErrorAction SilentlyContinue) }

function Find-Vulkan {
    $roots = @($env:VULKAN_SDK, "C:\VulkanSDK", "$env:ProgramFiles\VulkanSDK", "${env:ProgramFiles(x86)}\VulkanSDK")
    foreach ($root in $roots) {
        if ([string]::IsNullOrWhiteSpace($root) -or -not (Test-Path $root)) { continue }
        $items = @(Get-Item $root -ErrorAction SilentlyContinue) + @(Get-ChildItem $root -Directory -ErrorAction SilentlyContinue)
        foreach ($item in $items) {
            if ((Test-Path (Join-Path $item.FullName "Include\vulkan\vulkan.h")) -and
                (Test-Path (Join-Path $item.FullName "Lib\vulkan-1.lib"))) { return $item.FullName }
        }
    }
    return $null
}

function Install-CMake {
    if (-not (Has "winget")) { Fail "CMake is missing. Install CMake or use -InstallMissingTools on a PC with winget." }
    winget install --id Kitware.CMake --exact --source winget --accept-source-agreements --accept-package-agreements
    if ($LASTEXITCODE -ne 0) { Fail "CMake installation failed. Restart PowerShell and rerun." }
}

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $ProjectRoot) { $ProjectRoot = (Resolve-Path (Join-Path $scriptRoot "..\..")).Path }
$ProjectRoot = (Resolve-Path $ProjectRoot).Path
$source = Join-Path $ProjectRoot "Maaz_DriversUltimatePRO\MaazXGPU"
if (Test-Path (Join-Path $ProjectRoot "CMakeLists.txt")) { $source = $ProjectRoot }
if (-not (Test-Path (Join-Path $source "CMakeLists.txt"))) { Fail "CMakeLists.txt not found. Run this file from the repository or pass -ProjectRoot." }

if (-not (Has "cmake")) {
    if ($InstallMissingTools) { Install-CMake; Fail "CMake was installed. Open a new Developer PowerShell and rerun this script." }
    Fail "CMake is missing. Install CMake, then rerun. No large download is performed automatically."
}

$sdk = Find-Vulkan
if (-not $sdk) {
    Fail "Vulkan SDK is missing. Install LunarG Vulkan SDK, then rerun. Required: Include\vulkan\vulkan.h and Lib\vulkan-1.lib."
}
$env:VULKAN_SDK = $sdk
Info "Using Vulkan SDK: $sdk"

$generator = "Visual Studio 17 2022"
if (-not (Has "cl.exe")) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsPath) { Info "MSVC found at $vsPath; use Developer PowerShell if cl.exe is unavailable in this window." }
    }
    if (-not (Has "cl.exe")) { Fail "MSVC compiler is not active. Open 'Developer PowerShell for VS' and rerun this .ps1." }
}

$build = Join-Path $source "Build"
if ($Clean -and (Test-Path $build)) { Remove-Item $build -Recurse -Force }
Info "Configuring..."
cmake -B $build -S $source -G $generator -A x64
if ($LASTEXITCODE -ne 0) { Fail "CMake configuration failed." }
Info "Compiling Release..."
cmake --build $build --config Release --parallel
if ($LASTEXITCODE -ne 0) { Fail "Compilation failed." }

$out = Join-Path ([Environment]::GetFolderPath("Desktop")) "MaazXGPU-Driver"
New-Item $out -ItemType Directory -Force | Out-Null
$names = @("MaazXGPU.dll", "MaazXGPU.lib", "MaazXGPU_Tray.exe")
foreach ($name in $names) {
    $file = Get-ChildItem $build -Recurse -File -Filter $name | Select-Object -First 1
    if (-not $file) { Fail "$name was not produced by the build." }
    Copy-Item $file.FullName (Join-Path $out $name) -Force
}
foreach ($name in @("driver.inf", "README.txt", "BUILD-INSTRUCTIONS.md")) {
    $file = Join-Path $source $name
    if (Test-Path $file) { Copy-Item $file $out -Force }
}
if ($PackageZip) {
    $zip = Join-Path ([Environment]::GetFolderPath("Desktop")) "MaazXGPU-Driver.zip"
    if (Test-Path $zip) { Remove-Item $zip -Force }
    Compress-Archive -Path (Join-Path $out "*") -DestinationPath $zip -Force
    Info "ZIP created: $zip"
}
Info "SUCCESS: $out"
Get-ChildItem $out | Select-Object Name, Length
if (-not $NoPause) { Read-Host "Press Enter to close" | Out-Null }
