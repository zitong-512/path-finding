$ErrorActionPreference = 'Stop'
$project = $PSScriptRoot
$build = Join-Path $project '.build'
New-Item -ItemType Directory -Force -Path $build | Out-Null
$exe = Join-Path $build 'pathfinding.exe'
$sources = @(
    (Join-Path $project 'main.cpp')
)

$compiler = Get-Command g++, clang++, cl -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $compiler) {
    throw 'No C++ compiler found. Install MinGW-w64, LLVM, or MSVC and add it to PATH.'
}

Push-Location $build
try {
    if ($compiler.Name -eq 'cl.exe' -or $compiler.Name -eq 'cl') {
        & $compiler.Source /nologo /EHsc /std:c++17 /W4 /Fe:$exe $sources
    } else {
        & $compiler.Source -std=c++17 -Wall -Wextra -pedantic -g @sources -o $exe
    }
    $compileExitCode = $LASTEXITCODE
} finally {
    Pop-Location
}
if ($compileExitCode -ne 0) { exit $compileExitCode }

Push-Location $project
try {
    & $exe
    exit $LASTEXITCODE
} finally {
    Pop-Location
}
