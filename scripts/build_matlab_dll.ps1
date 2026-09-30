# build_matlab_dll.ps1
# Automated script to generate MATLAB parameters and build the C++ DLL
# Usage: .\scripts\build_matlab_dll.ps1 [-SkipParameterGeneration]

param(
    [switch]$SkipParameterGeneration = $false
)

Write-Host "Running MATLAB DLL build script..." -ForegroundColor Cyan
Write-Host ""

# Get the project root directory
$projectRoot = (Get-Location).Path
$matlabDir = Join-Path $projectRoot "matlab"

# Step 1: Generate parameters file (if not skipped)
if (-not $SkipParameterGeneration) {
    Write-Host "[Step 1/2] Generating parameters.mat..." -ForegroundColor Yellow
    & matlab -batch "cd('$matlabDir/build_data'); generate_parameters"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[+] Parameters file generated successfully!" -ForegroundColor Green
        Write-Host ""
    } else {
        Write-Host "[-] Failed to generate parameters file!" -ForegroundColor Red
        Write-Host ""
        exit 1
    }
} else {
    Write-Host "[Step 1/2] Skipping parameter generation (-SkipParameterGeneration flag set)" -ForegroundColor Yellow
    Write-Host ""
}

# Step 2: Build the DLL
Write-Host "[Step 2/2] Building MATLAB DLL..." -ForegroundColor Yellow
& matlab -batch "cd('$matlabDir'); build_dll"

if ($LASTEXITCODE -eq 0) {
    Write-Host "[+] MATLAB DLL built successfully" -ForegroundColor Green
    Write-Host ""
} else {
    Write-Host "[-] Failed to build MATLAB DLL" -ForegroundColor Red
    Write-Host ""
    exit 1
}

Write-Host "[+] MATLAB build process completed successfully!" -ForegroundColor Green
Write-Host ""
