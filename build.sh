#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
platform="windows"
target="all"
configure_only=false
clean_first=false
jobs=""

usage() {
    cat <<'EOF'
Usage: ./build.sh [options]

Configure and build Box3dTest for Windows by default.

Options:
  --game             Build only Box3dGame
  --editor           Build only Box3dEditor
  --all              Build all default targets (default)
  --windows          Cross-compile for 64-bit Windows (default)
  --native           Build for the current host instead
  --configure-only   Configure without compiling
  --clean-first      Clean the selected targets before building
  -j, --jobs N       Run N build jobs in parallel
  -h, --help         Show this help
EOF
}

while (($# > 0)); do
    case "$1" in
        --game)
            target="game"
            ;;
        --editor)
            target="editor"
            ;;
        --all)
            target="all"
            ;;
        --windows)
            platform="windows"
            ;;
        --native)
            platform="native"
            ;;
        --configure-only)
            configure_only=true
            ;;
        --clean-first)
            clean_first=true
            ;;
        -j|--jobs)
            if (($# < 2)) || [[ ! "$2" =~ ^[1-9][0-9]*$ ]]; then
                echo "Error: $1 requires a positive integer." >&2
                usage >&2
                exit 2
            fi
            jobs="$2"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Error: unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
    shift
done

command -v cmake >/dev/null 2>&1 || {
    echo "Error: cmake is not installed or not on PATH." >&2
    exit 1
}

if [[ "$platform" == "windows" ]]; then
    for command_name in x86_64-w64-mingw32-gcc x86_64-w64-mingw32-g++ x86_64-w64-mingw32-windres; do
        if ! command -v "$command_name" >/dev/null 2>&1; then
            echo "Error: Windows cross-compiler command not found: $command_name" >&2
            exit 1
        fi
    done
    configure_preset="windows-dev"
    if [[ "$target" == "all" ]]; then
        build_preset="windows-dev"
    else
        build_preset="windows-$target"
    fi
else
    configure_preset="dev"
    if [[ "$target" == "all" ]]; then
        build_preset="dev"
    else
        build_preset="$target"
    fi
fi

cmake -S "$script_dir" --preset "$configure_preset"

if [[ "$configure_only" == true ]]; then
    exit 0
fi

build_args=(--build --preset "$build_preset")

if [[ "$clean_first" == true ]]; then
    build_args+=(--clean-first)
fi

if [[ -n "$jobs" ]]; then
    build_args+=(--parallel "$jobs")
fi

cmake "${build_args[@]}"
