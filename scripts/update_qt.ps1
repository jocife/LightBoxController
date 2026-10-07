<#
.SYNOPSIS
    Build and run the Qt LightBox Controller application.

.DESCRIPTION
    This script builds the C++ Qt project and runs the resulting executable.
    Supports both Debug and Release configurations.

.PARAMETER Configuration
    Build configuration to use: 'Debug' or 'Release'.
    Default is 'Debug'.

.PARAMETER NoRun
    If specified, only builds the application without running it.

.EXAMPLE
    .\update_qt.ps1
    Builds and runs the Debug version.

.EXAMPLE
    .\update_qt.ps1 -Configuration Release
    Builds and runs the Release version.

.EXAMPLE
    .\update_qt.ps1 -Configuration Release -NoRun
    Builds the Release version without running it.
#>

param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [switch]$SkipConfigure,
    [switch]$NoRun
)

Write-Host "Running Qt build & run script..." -ForegroundColor Cyan
Write-Host ""

$projectRoot = (Get-Location).Path
$exePath = Join-Path $projectRoot "bin\$Configuration\LightBoxController.exe"
$buildDir = Join-Path $projectRoot "build"

# Initialize Visual Studio environment if not already set
if (-not $env:VCINSTALLDIR) {
    Write-Host "Initializing Visual Studio environment..." -ForegroundColor Cyan
    $vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2>$null
    if ($vsPath) {
        $vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
        if (Test-Path $vcvarsPath) {
            & cmd /c "`"$vcvarsPath`" && set" | ForEach-Object {
                if ($_ -match "=") {
                    $name, $value = $_.split("=", 2)
                    Set-Item -Path "env:$name" -Value $value
                }
            }
        }
    }
    Write-Host ""
}

$outputDir = Join-Path $projectRoot "bin\$Configuration"

Write-Host "Cleaning output directories..." -ForegroundColor Yellow
if (Test-Path $outputDir) {
    Remove-Item $outputDir -Recurse -Force
}
if (-not $SkipConfigure) {
    if (Test-Path $buildDir) {
        Remove-Item $buildDir -Recurse -Force
    }
}

if (-not $SkipConfigure) {
    Write-Host "Configuring CMake project..." -ForegroundColor Yellow
    cmake -S $projectRoot -B $buildDir
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[-] CMake configuration failed." -ForegroundColor Red
        exit 1
    }
}

# Build the project
Write-Host "[Step 1/2] Building C++ project ($Configuration)..." -ForegroundColor Yellow
cmake --build $buildDir --config $Configuration
if ($LASTEXITCODE -ne 0) {
    Write-Host "[-] Build failed." -ForegroundColor Red
    exit 1
}

Write-Host ""

# Run the application (unless -NoRun flag is set)
if (-not $NoRun) {
    Write-Host "[Step 2/2] Running application..." -ForegroundColor Yellow
    Write-Host ""
    if (Test-Path $exePath) {
        & $exePath
    } else {
        Write-Host "[-] Executable not found: $exePath" -ForegroundColor Red
        Write-Host ""
        exit 1
    }
} else {
    Write-Host "[Step 2/2] Build complete (skipping run due to -NoRun flag)" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "[+] Executable ready at: $exePath" -ForegroundColor Green
}

Write-Host ""
