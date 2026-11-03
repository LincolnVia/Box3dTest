#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
target="dev"
configure_only=false
clean_first=false
jobs=""

usage() {
    cat <<'EOF'
Usage: ./build-mac.sh [options]

Configure and build for macOS using the native CMake toolchain.

Options:
  --game             Build only Box3dGame
  --editor           Build only Box3dEditor
  --all              Build all default targets (default)
  --configure-only   Configure without compiling
  --clean-first      Clean the selected targets before building
  -j, --jobs N       Run N build jobs in parallel
  -h, --help         Show this help
EOF
}

while (($# > 0)); do
    case "$1" in
        --game) target="game" ;;
        --editor) target="editor" ;;
        --all) target="dev" ;;
        --configure-only) configure_only=true ;;
        --clean-first) clean_first=true ;;
        -j|--jobs)
            if (($# < 2)) || [[ ! "$2" =~ ^[1-9][0-9]*$ ]]; then
                echo "Error: $1 requires a positive integer." >&2
                usage >&2
                exit 2
            fi
            jobs="$2"
            shift
            ;;
        -h|--help) usage; exit 0 ;;
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

cmake -S "$script_dir" --preset dev

if [[ "$configure_only" == true ]]; then
    exit 0
fi

build_args=(--build --preset "$target")
if [[ "$clean_first" == true ]]; then
    build_args+=(--clean-first)
fi
if [[ -n "$jobs" ]]; then
    build_args+=(--parallel "$jobs")
fi

cmake "${build_args[@]}"
