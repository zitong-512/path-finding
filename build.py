"""Discover, compile, and run the pathfinding algorithms on any platform."""

from pathlib import Path
import os
import re
import shutil
import subprocess


def build(project: Path) -> None:
    build_directory = project / ".build"
    build_directory.mkdir(exist_ok=True)
    pattern = re.compile(
        r"class\s+([A-Za-z_][A-Za-z_0-9]*)\s*(?:final\s*)?:\s*public\s+Algorithm\s*<\s*T\s*>"
    )
    algorithms = []
    for header in sorted((project / "Algorithms").glob("*.hpp")):
        matches = pattern.findall(header.read_text(encoding="utf-8-sig"))
        if not matches:
            continue
        if len(matches) != 1:
            raise RuntimeError(f"Expected one Algorithm<T> subclass in {header.name}")
        name = matches[0]
        if name != header.stem:
            raise RuntimeError(f"{header.name} defines {name}. Name the class {header.stem} to register it.")
        algorithms.append((name, header.name, name.lower()))
    if not algorithms:
        raise RuntimeError("No Algorithm<T> subclasses found in Algorithms")
    if len({item[2] for item in algorithms}) != len(algorithms):
        raise RuntimeError("Algorithm class names must be unique, ignoring case")

    registry = ['#pragma once', '#include "../Algorithms/Algorithm.hpp"']
    registry.extend(f'#include "../Algorithms/{header}"' for _, header, _ in algorithms)
    registry.extend([
        '#include <memory>', '#include <string>', '#include <utility>', '#include <vector>',
        'template <typename T>',
        'std::vector<std::pair<std::string, std::unique_ptr<Algorithm<T>>>> makeAlgorithms() {',
        '    std::vector<std::pair<std::string, std::unique_ptr<Algorithm<T>>>> algorithms;',
    ])
    registry.extend(
        f'    algorithms.emplace_back("{identifier}", std::make_unique<{name}<T>>());'
        for name, _, identifier in algorithms
    )
    registry.extend(['    return algorithms;', '}'])
    (build_directory / "AlgorithmRegistry.hpp").write_text("\n".join(registry) + "\n", encoding="utf-8")

    compiler = next((path for name in ("g++", "clang++", "cl") if (path := shutil.which(name))), None)
    if compiler is None:
        raise RuntimeError(
            "No C++17 compiler found on PATH. On macOS run: xcode-select --install. "
            "On Windows install MinGW-w64, LLVM, or MSVC; on Linux install g++ or clang++."
        )
    executable = build_directory / ("pathfinding.exe" if os.name == "nt" else "pathfinding")
    source = str(project / "main.cpp")
    if Path(compiler).stem.lower() == "cl":
        command = [compiler, "/nologo", "/EHsc", "/std:c++17", "/W4", f"/Fe:{executable}", source]
    else:
        command = [compiler, "-std=c++17", "-Wall", "-Wextra", "-pedantic", "-g", source, "-o", str(executable)]
    subprocess.run(command, cwd=build_directory, check=True)
    (project / "web").mkdir(exist_ok=True)
    subprocess.run([str(executable)], cwd=project, check=True)


if __name__ == "__main__":
    try:
        build(Path(__file__).resolve().parent)
    except (RuntimeError, OSError, subprocess.CalledProcessError) as error:
        raise SystemExit(f"Build failed: {error}")
