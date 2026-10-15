#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 [repository-root] [--build]" >&2
}

repo_root="${1:-.}"
build_requested=false

if [[ "$repo_root" == "--build" ]]; then
    repo_root="."
    build_requested=true
elif [[ "${2:-}" == "--build" ]]; then
    build_requested=true
elif [[ -n "${2:-}" ]]; then
    usage
    exit 2
fi

repo_root="$(cd "$repo_root" && pwd)"

for command_name in cmake x86_64-w64-mingw32-gcc x86_64-w64-mingw32-g++ x86_64-w64-mingw32-windres; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        echo "Missing required command: $command_name" >&2
        exit 1
    fi
done

if [[ ! -f "$repo_root/CMakeLists.txt" ]] || \
   ! grep -Eq 'project\([[:space:]]*Box3dTest' "$repo_root/CMakeLists.txt"; then
    echo "Not a Box3dTest repository: $repo_root" >&2
    exit 1
fi

if [[ ! -f "$repo_root/CMakePresets.json" ]]; then
    echo "Missing CMakePresets.json in $repo_root" >&2
    exit 1
fi

required_sources=()

for source_path in "${required_sources[@]}"; do
    if [[ ! -f "$repo_root/$source_path" ]]; then
        echo "Required project source is missing: $source_path" >&2
        echo "Preserving the working tree; restore or replace that file before configuring." >&2
        exit 1
    fi
done

cmake --preset windows-dev -S "$repo_root"

if [[ "$build_requested" == true ]]; then
    cmake --build --preset windows-dev
fi

echo "Box3dTest development environment is ready."
echo "Compile database: $repo_root/build/windows-dev/compile_commands.json"
