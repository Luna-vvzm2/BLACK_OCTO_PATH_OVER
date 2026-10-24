param([int]$Port = 8085)
$ErrorActionPreference = 'Stop'
if (!(Test-Path (Join-Path $PSScriptRoot 'build/index.html'))) { throw 'Run build.ps1 first.' }
Write-Host "Open http://127.0.0.1:$Port/ . Ctrl+C stops the server."
python -m http.server $Port --bind 127.0.0.1 --directory (Join-Path $PSScriptRoot 'build')
