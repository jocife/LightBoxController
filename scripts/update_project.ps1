<#
.SYNOPSIS
    Complete project update workflow: build MATLAB DLL and compile/run Qt application.

.DESCRIPTION
    This script orchestrates the full project update process:
    1. Builds MATLAB DLL and copies files to project
    2. Rebuilds the C++ Qt application
    3. Runs the compiled executable

.PARAMETER Configuration
    Build configuration to use: 'Debug' or 'Release'.
    Default is 'Debug'.

.PARAMETER SkipMatlabBuild
    If specified, skips the MATLAB build step.
    Useful when only rebuilding the Qt application.

.PARAMETER NoRun
    If specified, builds the application without running it.

.PARAMETER MatlabRootDir
    Path to the MATLAB installation root directory.
    If not specified, automatically detects the latest MATLAB version.
    Example: -MatlabRootDir "D:/MathWorks/MATLAB/R2024a"

.EXAMPLE
    .\update_project.ps1
    Runs the complete workflow: builds MATLAB DLL, Qt app, and runs it (Debug).

.EXAMPLE
    .\update_project.ps1 -Configuration Release
    Builds and runs the Release version.

.EXAMPLE
    .\update_project.ps1 -SkipMatlabBuild
    Skips MATLAB build and only rebuilds/runs the Qt application.

.EXAMPLE
    .\update_project.ps1 -Configuration Release -NoRun
    Builds both MATLAB and Qt (Release) without running the app.
#>

param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [switch]$SkipMatlabBuild,
    [switch]$NoRun,
    [string]$MatlabRootDir
)

Write-Host "Running complete project update workflow..." -ForegroundColor Cyan
Write-Host ""

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir

# Step 1: Build MATLAB DLL (if not skipped)
if (-not $SkipMatlabBuild) {
    & "$scriptDir\update_matlab.ps1" -MatlabRootDir:$MatlabRootDir
}

# Step 2: Build and run Qt application
& "$scriptDir\update_qt.ps1" -Configuration $Configuration -NoRun:$NoRun
