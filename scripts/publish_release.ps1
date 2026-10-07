<#
.SYNOPSIS
    Builds the Release installer and publishes it to GitHub.

.DESCRIPTION
    This script runs the build_release_exe.ps1 script to ensure a fresh installer is built.
    It then uses the GitHub CLI (gh) to create a new release and upload the generated installer.

.PARAMETER Tag
    The git tag to create the release for (e.g., v1.1.0).

.PARAMETER SkipBuild
    If specified, skips building the installer and only publishes the existing one.
#>
param(
    [Parameter(Mandatory=$true)]
    [string]$Tag,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$projectRoot = (Get-Location).Path

# 1. Verify GitHub CLI is installed
if (-not (Get-Command "gh" -ErrorAction SilentlyContinue)) {
    throw "GitHub CLI ('gh') is not installed. Please install it (winget install --id GitHub.cli) and run 'gh auth login' before publishing."
}

# 2. Extract version from CMakeLists.txt to find the installer
$cmakeFile = Join-Path $projectRoot "CMakeLists.txt"
$version = "1.0.0" # Default fallback
$cmakeContent = Get-Content $cmakeFile -Raw
if ($cmakeContent -match 'set\(LightBoxController_VERSION\s+"([^"]+)"') {
    $version = $matches[1]
}

$installerPath = Join-Path $projectRoot "build\LightBoxController-$version-win64.exe"

# 3. Build the installer if not skipped
if (-not $SkipBuild) {
    Write-Host "Building Release executable and installer..." -ForegroundColor Cyan
    $buildScript = Join-Path $projectRoot "scripts\build_release_exe.ps1"
    & $buildScript
}

if (-not (Test-Path $installerPath)) {
    throw "Installer not found at expected path: $installerPath"
}

# 4. Publish to GitHub
Write-Host "Publishing release $Tag to GitHub..." -ForegroundColor Cyan
gh release create $Tag $installerPath --title "Release $Tag" --generate-notes

Write-Host "Successfully published release $Tag!" -ForegroundColor Green

