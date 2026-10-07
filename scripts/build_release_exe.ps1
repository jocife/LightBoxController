<#
.SYNOPSIS
    Builds the LightBox Controller Release executable from source.

.DESCRIPTION
    Configures and builds the Qt/CMake project, then deletes any stale Release output
    directory before generating a fresh executable in bin/Release.

.PARAMETER Configuration
    Build configuration to use. Supported values are 'Debug' and 'Release'.
    Default is 'Release'.

.PARAMETER SkipConfigure
    If specified, skips the CMake configure step and only builds the project.

.PARAMETER SkipInstaller
    If specified, skips generating the Windows NSIS installer package.

.EXAMPLE
    .\build_release_exe.ps1
    Builds the Release executable and creates the NSIS installer by default.

.EXAMPLE
    .\build_release_exe.ps1 -SkipInstaller
    Builds the Release executable without generating the NSIS installer.
#>

param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [switch]$SkipConfigure,
    [switch]$SkipInstaller
)

$ErrorActionPreference = "Stop"

Write-Host "===== LightBox Controller Release Build =====" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
Write-Host ""

$projectRoot = (Get-Location).Path
$buildDir = Join-Path $projectRoot "build"
$releaseOutputDir = Join-Path $projectRoot "bin\$Configuration"
$exePath = Join-Path $releaseOutputDir "LightBoxController.exe"

# Initialize Visual Studio build environment if needed.
if (-not $env:VCINSTALLDIR) {
    Write-Host "Initializing Visual Studio environment..." -ForegroundColor Cyan
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"

    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
        if ($vsPath) {
            $vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
            if (Test-Path $vcvarsPath) {
                & cmd /c "`"$vcvarsPath`" && set" | ForEach-Object {
                    if ($_ -match "^([^=]+)=(.*)$") {
                        $name = $matches[1]
                        $value = $matches[2]
                        Set-Item -Path "env:$name" -Value $value
                    }
                }
            }
        }
    }

    if (-not $env:VCINSTALLDIR) {
        Write-Warning "Visual Studio environment was not detected. Ensure the C++ build tools are available before building."
    }
}

Write-Host "[1/3] Removing stale output directories..." -ForegroundColor Yellow
if (Test-Path $releaseOutputDir) {
    Remove-Item $releaseOutputDir -Recurse -Force
}
if (-not $SkipConfigure) {
    if (Test-Path $buildDir) {
        Get-ChildItem -Path $buildDir -Force -Exclude ".gitkeep" | Remove-Item -Recurse -Force
    }
}

Write-Host "[2/3] Configuring CMake project..." -ForegroundColor Yellow
if (-not $SkipConfigure) {
    cmake -S $projectRoot -B $buildDir
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed."
    }
} else {
    Write-Host "Skipping CMake configure step as requested." -ForegroundColor DarkGray
}

Write-Host "[3/3] Building application ($Configuration) ..." -ForegroundColor Yellow
cmake --build $buildDir --config $Configuration --target LightBoxController
if ($LASTEXITCODE -ne 0) {
    throw "Build failed."
}

if (Test-Path $exePath) {
    Write-Host "" 
    Write-Host "[+] Executable generated successfully: $exePath" -ForegroundColor Green
} else {
    throw "Executable was not found at expected path: $exePath"
}

if (-not $SkipInstaller) {
    Write-Host "" 
    Write-Host "Creating NSIS installer package..." -ForegroundColor Yellow
    Push-Location $buildDir
    cpack -C $Configuration -G NSIS
    if ($LASTEXITCODE -ne 0) {
        Pop-Location
        throw "NSIS packaging failed."
    }
    Pop-Location
    Write-Host "[+] Installer package created in $buildDir" -ForegroundColor Green
} else {
    Write-Host "" 
    Write-Host "[+] NSIS installer generation skipped by request." -ForegroundColor DarkGray
}

Write-Host "" 
Write-Host "Build script completed successfully." -ForegroundColor Cyan
