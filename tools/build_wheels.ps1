[CmdletBinding()]
param(
    [ValidateSet("AMD64")]
    [string]$Architecture = "AMD64",

    [ValidateSet("Auto", "MSVC", "MinGW")]
    [string]$Toolchain = "Auto",

    [string]$Build = "",

    [string]$Python = "python",

    [switch]$SkipBootstrap
)

# Build native Windows wheels for hightorque-robot.
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File tools/build_wheels.ps1
#
# By default, build the Python 3.8-3.14 win_amd64 matrix from pyproject.toml.
# Requires 64-bit Windows 10, Python 3.11+, and MSVC or MinGW-w64.

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = [IO.Path]::GetFullPath((Join-Path $scriptDir ".."))
$projectDir = [IO.Path]::GetFullPath((Join-Path $scriptDir "python"))
$coreDir = [IO.Path]::GetFullPath((Join-Path $projectDir "core"))
$distDir = [IO.Path]::GetFullPath((Join-Path $scriptDir "dist"))
$venvDir = [IO.Path]::GetFullPath((Join-Path $scriptDir ".cibw-venv-windows"))
$venvPython = Join-Path $venvDir "Scripts/python.exe"
$cibwCacheDir = Join-Path $venvDir "cibuildwheel-cache"

function Test-MsvcAvailable {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        return $true
    }

    $vswhereCandidates = @(
        (Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio/Installer/vswhere.exe"),
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio/Installer/vswhere.exe")
    )
    foreach ($candidate in $vswhereCandidates) {
        if (Test-Path -LiteralPath $candidate) {
            $installation = & $candidate -latest -products * `
                -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
                -property installationPath
            if ($installation) {
                return $true
            }
        }
    }
    return $false
}

function Test-MinGWAvailable {
    $hasCompiler = [bool](Get-Command g++.exe -ErrorAction SilentlyContinue)
    $hasMake = [bool](Get-Command mingw32-make.exe -ErrorAction SilentlyContinue)
    return $hasCompiler -and $hasMake
}

if ($Toolchain -eq "Auto") {
    if (Test-MsvcAvailable) {
        $Toolchain = "MSVC"
    }
    elseif (Test-MinGWAvailable) {
        $Toolchain = "MinGW"
    }
    else {
        throw "No supported C++ toolchain found. Install MSVC or MinGW-w64."
    }
}

if ($Toolchain -eq "MSVC" -and -not (Test-MsvcAvailable)) {
    throw "MSVC not found. Install Visual Studio 2022 C++ Build Tools."
}
if ($Toolchain -eq "MinGW") {
    if (-not (Test-MinGWAvailable)) {
        throw "g++.exe or mingw32-make.exe not found. Add MinGW-w64 to PATH."
    }
    $env:CMAKE_GENERATOR = "MinGW Makefiles"
}

Write-Host "==> Windows C++ toolchain: $Toolchain"

if (-not $coreDir.StartsWith($projectDir + [IO.Path]::DirectorySeparatorChar,
                            [StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to use a core directory outside the Python project: $coreDir"
}

Write-Host "==> staging core sources -> $coreDir"
New-Item -ItemType Directory -Force -Path $coreDir | Out-Null
& robocopy $repoRoot $coreDir /MIR `
    /XD (Join-Path $repoRoot ".git") `
        (Join-Path $repoRoot "build") `
        (Join-Path $repoRoot "tools") `
    /XF "*.pyc" /NFL /NDL /NJH /NJS /NP
$robocopyCode = $LASTEXITCODE
if ($robocopyCode -ge 8) {
    throw "robocopy staging failed with exit code $robocopyCode"
}
$global:LASTEXITCODE = 0

if (-not (Test-Path -LiteralPath $venvPython)) {
    Write-Host "==> creating Windows build environment: $venvDir"
    & $Python -m venv $venvDir
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to create a virtual environment with '$Python'"
    }
}

if (-not $SkipBootstrap) {
    Write-Host "==> installing wheel build dependencies"
    & $venvPython -m pip install --upgrade pip
    if ($LASTEXITCODE -ne 0) { throw "Failed to upgrade pip" }

    $cibuildwheelSpec = if ($env:CIBW_VERSION_SPEC) { $env:CIBW_VERSION_SPEC } else { "cibuildwheel<4" }
    & $venvPython -m pip install --upgrade $cibuildwheelSpec
    if ($LASTEXITCODE -ne 0) { throw "Failed to install cibuildwheel" }
}

New-Item -ItemType Directory -Force -Path $distDir, $cibwCacheDir | Out-Null
$env:CIBW_CACHE_PATH = $cibwCacheDir
$env:CIBW_ARCHS_WINDOWS = $Architecture
$env:CIBW_TEST_REQUIRES = "pytest"
if ($Build) {
    $env:CIBW_BUILD = $Build
}

$buildDisplay = if ($Build) { $Build } else { "pyproject default" }
Write-Host "==> building Windows wheels (arch=$Architecture, build=$buildDisplay)"
& $venvPython -m cibuildwheel $projectDir `
    --platform windows `
    --output-dir $distDir
if ($LASTEXITCODE -ne 0) {
    throw "Windows wheel build failed"
}

Write-Host "==> done:"
Get-ChildItem -LiteralPath $distDir -Filter "hightorque_robot-*-win_amd64.whl" |
    Select-Object Name, Length, LastWriteTime
