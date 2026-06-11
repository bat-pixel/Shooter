# Build Sky Fire Patrol as a WebAssembly app using Emscripten.
#
# Prerequisites:
#   Emscripten SDK installed and the environment activated.
#   In a fresh terminal run:
#       & "$env:EMSDK\emsdk_env.ps1"   (or wherever your emsdk lives)
#   Then run this script from the Shooter\Shooter\ directory.
#
# Output: build_web\index.html + index.js + index.wasm + index.data
# Serve locally:
#   python -m http.server 8080 --directory build_web

$ErrorActionPreference = 'Stop'
$ScriptDir = $PSScriptRoot
$BuildDir  = Join-Path $ScriptDir 'build_web'

# Bootstrap Emscripten environment if emcc is not already on PATH
if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
    $EmsdkRoot = if ($env:EMSDK) { $env:EMSDK } else { 'C:\emsdk' }
    $env:PATH        = "$EmsdkRoot;$EmsdkRoot\upstream\emscripten;" + $env:PATH
    $env:EMSDK       = $EmsdkRoot.Replace('\', '/')
    $env:EMSDK_NODE  = "$EmsdkRoot\node\22.16.0_64bit\bin\node.exe"
    $env:EMSDK_PYTHON= "$EmsdkRoot\python\3.13.3_64bit\python.exe"
    Write-Host "  (Loaded emsdk from $EmsdkRoot)" -ForegroundColor DarkGray
}

Write-Host '==> Configuring with emcmake ...' -ForegroundColor Cyan
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
Push-Location $BuildDir

try {
    emcmake cmake $ScriptDir -DCMAKE_BUILD_TYPE=Release
    Write-Host '==> Building ...' -ForegroundColor Cyan
    emmake cmake --build . --parallel
    Write-Host ''
    Write-Host "Done!  Files are in: $BuildDir" -ForegroundColor Green
    Write-Host ''
    Write-Host 'To play locally:'
    Write-Host "  python -m http.server 8080 --directory `"$BuildDir`""
    Write-Host '  Then open http://localhost:8080'
} finally {
    Pop-Location
}
