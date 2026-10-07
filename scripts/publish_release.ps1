<#
.SYNOPSIS
    Builds the Release installer and publishes it to GitHub based on the latest Git tag.

.DESCRIPTION
    This script runs the build_release_exe.ps1 script to ensure a fresh installer is built.
    It automatically reads the latest Git tag to determine the version, creates a new GitHub
    release using the GitHub CLI (gh), and uploads the generated installer.

.PARAMETER Tag
    Optional. The git tag to create the release for. If omitted, the latest git tag is used automatically.

.PARAMETER SkipBuild
    If specified, skips building the installer and only publishes the existing one.
#>
param(
    [string]$Tag,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$projectRoot = (Get-Location).Path

# 1. Verify GitHub CLI is installed
if (-not (Get-Command "gh" -ErrorAction SilentlyContinue)) {
    throw "GitHub CLI ('gh') is not installed. Please install it (winget install --id GitHub.cli) and run 'gh auth login' before publishing."
}

# 2. Get the target release tag from Git
if (-not $Tag) {
    $Tag = git describe --tags --abbrev=0
    if ($LASTEXITCODE -ne 0 -or -not $Tag) {
        throw "Could not determine the latest git tag. Ensure you have created a tag (e.g. 'git tag v1.1.1')."
    }
    $Tag = $Tag.Trim()
}

Write-Host "Target Release Tag: $Tag" -ForegroundColor Yellow

$installerPath = Join-Path $projectRoot "build\LightBoxController-$Tag-win64.exe"

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
