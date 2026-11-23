$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $compiler = Join-Path $PSScriptRoot 'tools/emsdk/upstream/emscripten/em++.exe'
    if (!(Test-Path -LiteralPath $compiler)) { throw 'Install the SDK using setup.ps1 first.' }
    New-Item -ItemType Directory -Path 'build' -Force | Out-Null
    & $compiler main.cpp -std=c++17 -Wall -Wextra -Werror -O2 --shell-file shell.html -o build/index.html
    if ($LASTEXITCODE -ne 0) { throw 'Emscripten build failed.' }
    Write-Host 'Build complete. Run serve.ps1 and open http://127.0.0.1:8085/'
} finally { Pop-Location }
