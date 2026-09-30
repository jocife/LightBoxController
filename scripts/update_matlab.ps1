<#
.SYNOPSIS
    Complete MATLAB update workflow: build DLL and copy files to project.

.DESCRIPTION
    This script orchestrates the full MATLAB DLL generation and deployment process:
    1. Generates parameters.mat with reference data
    2. Builds C++ DLLs from MATLAB code using MATLAB Coder
    3. Copies generated files and dependencies to the Qt project

.PARAMETER SkipParameterGeneration
    If specified, skips the parameters.mat generation step.
    Useful when only rebuilding the DLL without changing reference data.

.PARAMETER MatlabRootDir
    Path to the MATLAB installation root directory.
    If not specified, automatically detects the latest MATLAB version.
    Example: -MatlabRootDir "D:/MathWorks/MATLAB/R2024a"

.EXAMPLE
    .\update_matlab.ps1
    Runs the complete workflow with automatic MATLAB detection.

.EXAMPLE
    .\update_matlab.ps1 -SkipParameterGeneration
    Rebuilds DLLs without regenerating parameters.mat.

.EXAMPLE
    .\update_matlab.ps1 -MatlabRootDir "D:/MathWorks/MATLAB/R2024a"
    Runs the complete workflow with a specific MATLAB installation.
#>

param(
    [switch]$SkipParameterGeneration,
    [string]$MatlabRootDir
)

Write-Host "Running MATLAB update workflow script..." -ForegroundColor Cyan
Write-Host ""

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir

# Step 1: Build MATLAB DLL
$buildScriptArgs = @()
if ($SkipParameterGeneration) {
    $buildScriptArgs += "-SkipParameterGeneration"
}
if ($MatlabRootDir) {
    $buildScriptArgs += "-MatlabRootDir", $MatlabRootDir
}

& "$scriptDir\build_matlab_dll.ps1" @buildScriptArgs

# Step 2: Copy files to project
$copyScriptArgs = @()
if ($MatlabRootDir) {
    $copyScriptArgs += "-MatlabRootDir", $MatlabRootDir
}

& "$scriptDir\copy_matlab_files.ps1" @copyScriptArgs

Write-Host "[+] MATLAB update workflow completed successfully!" -ForegroundColor Green
Write-Host ""
