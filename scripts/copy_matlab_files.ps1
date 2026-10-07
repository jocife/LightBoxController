[CmdletBinding()]
param (
    [string]$MatlabRootDir = ""
)

Write-Host "Running MATLAB file copy script..." -ForegroundColor Cyan
Write-Host ""

# Run this script from the root directory of the LightBoxController project
$ProjectRootDir = (Get-Location).Path
$MatlabCodegenDir = Join-Path $ProjectRootDir "matlab\codegen\dll\optimize_led_weights"

$IncludeDir = Join-Path $ProjectRootDir "include"
$LibDir = Join-Path $ProjectRootDir "lib"

# Ensure destination directories exist without deleting other potential contents
if (-not (Test-Path $IncludeDir)) { New-Item -Path $IncludeDir -ItemType Directory | Out-Null }
if (-not (Test-Path $LibDir)) { New-Item -Path $LibDir -ItemType Directory | Out-Null }

# Copy generated MATLAB files (overwrites existing files safely)
Write-Host "[Step 1/2] Copying generated MATLAB files..." -ForegroundColor Yellow

Copy-Item -Path (Join-Path $MatlabCodegenDir "*.h") -Destination $IncludeDir -Force
Copy-Item -Path (Join-Path $MatlabCodegenDir "*.lib") -Destination $LibDir -Force
Copy-Item -Path (Join-Path $MatlabCodegenDir "*.dll") -Destination $LibDir -Force

Write-Host "[+] Generated MATLAB files copied successfully!" -ForegroundColor Green
Write-Host ""

# Determine MATLAB root directory dynamically if not provided
if (-not $MatlabRootDir) {
    $MatlabBaseDir = "C:\Program Files\MATLAB"
    if (Test-Path $MatlabBaseDir) {
        # Get the latest MATLAB version folder
        $MatlabRootDir = (Get-ChildItem -Path $MatlabBaseDir -Directory | Sort-Object Name -Descending | Select-Object -First 1).FullName
    } else {
        Write-Error "[-] MATLAB installation not found in default path ($MatlabBaseDir). Please provide the path via -MatlabRootDir parameter."
        Write-Host ""
        exit 1
    }
}

Write-Host "Using MATLAB path: $MatlabRootDir" -ForegroundColor Cyan
Write-Host ""

# Copy MATLAB dependencies
Write-Host "[Step 2/2] Copying MATLAB dependencies..." -ForegroundColor Yellow
Copy-Item -Path (Join-Path $MatlabRootDir "extern\include\tmwtypes.h") -Destination $IncludeDir -Force
Copy-Item -Path (Join-Path $MatlabRootDir "bin\win64\libiomp5md.dll") -Destination $LibDir -Force

Write-Host "[+] MATLAB dependencies copied successfully!" -ForegroundColor Green
Write-Host ""
