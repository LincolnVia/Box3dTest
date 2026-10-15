---
name: box3d-environment
description: Set up and verify the local C++ development environment for the Box3dTest repository. Use for configuring, building, refreshing clangd metadata, or diagnosing project prerequisites; do not use for unrelated CMake projects.
---

# Box3dTest environment

Work from the Box3dTest repository root. Preserve existing user changes, especially missing or modified source files.

Run `scripts/setup.sh <repository-root>` from this skill directory. By default it checks prerequisites and configures the `windows-dev` CMake preset. Pass `--build` after the repository path when the user wants a compile verification. Use the repository's `build.sh`; it targets 64-bit Windows by default and accepts `--native` only when the user explicitly wants a host build.

The setup should leave these invariants in place:

- CMake uses the `windows-dev` preset and writes generated state to `build/windows-dev`.
- clangd reads `build/windows-dev/compile_commands.json` through the repository's `.clangd` file.
- The game and editor can be built separately with the `windows-game` and `windows-editor` build presets.
- Runtime resources remain copied by the project's existing post-build commands.

If a required tool is absent, report the exact missing command. The WSL workflow requires CMake, a CMake-supported build tool, and the `x86_64-w64-mingw32` GCC/G++/windres toolchain. Do not install system packages without authorization.

If configuration fails because a tracked project source is absent, identify it as a working-tree issue and do not restore or recreate it unless the user asks. If dependency retrieval fails because network access is restricted, request the required network authorization and retry once.
