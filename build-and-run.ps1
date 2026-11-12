$ErrorActionPreference = 'Stop'
$project = $PSScriptRoot
$build = Join-Path $project '.build'
New-Item -ItemType Directory -Force -Path $build | Out-Null
$exe = Join-Path $build 'pathfinding.exe'
$sources = @(
    (Join-Path $project 'main.cpp')
)

# Generate the C++ registry from concrete Algorithm<T> classes.
# Each class must be default-constructible and share its header's filename.
$algorithmDirectory = Join-Path $project 'Algorithms'
$headers = Get-ChildItem -LiteralPath $algorithmDirectory -Filter '*.hpp' -File | Sort-Object Name
$pattern = 'class\s+(?<name>[A-Za-z_][A-Za-z_0-9]*)\s*(?:final\s*)?:\s*public\s+Algorithm\s*<\s*T\s*>'
$algorithms = @()
foreach ($header in $headers) {
    $source = [System.IO.File]::ReadAllText($header.FullName)
    $matches = [regex]::Matches($source, $pattern)
    if ($matches.Count -eq 0) { continue }
    if ($matches.Count -ne 1) { throw "Expected one Algorithm<T> subclass in $($header.Name)" }
    $className = $matches[0].Groups['name'].Value
    if ($className -cne $header.BaseName) {
        throw "$($header.Name) defines $className. Name the class $($header.BaseName) to register it."
    }
    $algorithms += [pscustomobject]@{ ClassName = $className; Header = $header.Name; Id = $className.ToLowerInvariant() }
}
if ($algorithms.Count -eq 0) { throw 'No Algorithm<T> subclasses found in Algorithms' }
if (($algorithms | Select-Object -ExpandProperty Id | Sort-Object -Unique).Count -ne $algorithms.Count) {
    throw 'Algorithm class names must be unique, ignoring case'
}

$registry = New-Object 'System.Collections.Generic.List[string]'
$registry.Add('#pragma once')
$registry.Add('#include "../Algorithms/Algorithm.hpp"')
foreach ($algorithm in $algorithms) { $registry.Add('#include "../Algorithms/' + $algorithm.Header + '"') }
$registry.Add('#include <memory>')
$registry.Add('#include <string>')
$registry.Add('#include <utility>')
$registry.Add('#include <vector>')
$registry.Add('template <typename T>')
$registry.Add('std::vector<std::pair<std::string, std::unique_ptr<Algorithm<T>>>> makeAlgorithms() {')
$registry.Add('    std::vector<std::pair<std::string, std::unique_ptr<Algorithm<T>>>> algorithms;')
foreach ($algorithm in $algorithms) {
    $registry.Add('    algorithms.emplace_back("' + $algorithm.Id + '", std::make_unique<' + $algorithm.ClassName + '<T>>());')
}
$registry.Add('    return algorithms;')
$registry.Add('}')
[System.IO.File]::WriteAllLines((Join-Path $build 'AlgorithmRegistry.hpp'), $registry, [System.Text.UTF8Encoding]::new($false))

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
