$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    if (!(Test-Path 'tools/emsdk/emsdk.py')) {
        git clone --depth 1 https://github.com/emscripten-core/emsdk.git tools/emsdk
        if ($LASTEXITCODE -ne 0) { throw 'SDK download failed.' }
    }
    python tools/emsdk/emsdk.py install 6.0.13
    if ($LASTEXITCODE -ne 0) { throw 'SDK installation failed.' }
    python tools/emsdk/emsdk.py activate 6.0.13
    if ($LASTEXITCODE -ne 0) { throw 'SDK activation failed.' }
} finally { Pop-Location }
