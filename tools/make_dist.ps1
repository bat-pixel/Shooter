# Build a distributable zip of Skyfire Patrol
# Usage: .\tools\make_dist.ps1
# Output: dist\SkyFirePatrol_<version>.zip

$ErrorActionPreference = "Stop"
$root    = Split-Path $PSScriptRoot -Parent
$release = "$root\x64\Release"
$dist    = "$root\dist"

# --- 1. Build Release ---
Write-Host "Building Release|x64..." -ForegroundColor Cyan
$vstools = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
cmd /c "`"$vstools`" -arch=amd64 2>nul && msbuild `"$root\Shooter.vcxproj`" /p:Configuration=Release /p:Platform=x64 /v:minimal /t:Rebuild"
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed."; exit 1 }

# --- 2. Stage into a clean folder ---
$version  = (git -C $root log -1 --format="%h").Trim()
$stageName = "SkyFirePatrol_$version"
$stage    = "$dist\$stageName"

if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item -ItemType Directory -Force $stage | Out-Null

# Exe + DLLs
Copy-Item "$release\*.exe" $stage
Copy-Item "$release\*.dll" $stage

# Assets (skip intermediate build artefacts if any land there)
Copy-Item "$release\assets" $stage -Recurse

# --- 3. Zip ---
$zipPath = "$dist\$stageName.zip"
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Compress-Archive -Path "$stage\*" -DestinationPath $zipPath

# --- 4. Report ---
$sizeMB = [math]::Round((Get-Item $zipPath).Length / 1MB, 1)
Write-Host ""
Write-Host "Done: $zipPath ($sizeMB MB)" -ForegroundColor Green
Write-Host "Contents:" -ForegroundColor Gray
Get-ChildItem $stage | Select-Object -ExpandProperty Name | ForEach-Object { Write-Host "  $_" }
