# TrenchBroom Compile Profile

The `Compile` profile is configured in `VEngine/CompilationProfiles.cfg`. It exports a TrenchBroom-clean map, runs QBSP, VIS, and LIGHT, copies the BSP into the local build resources, and launches the game.

The macOS ericw-tools 2.0.0-alpha11 executables and their required runtime libraries are included in `tools/trenchbroom/ericw-tools/`. The profile resolves them from `${GAME_DIR_PATH}`, so it works with the project root selected as the TrenchBroom game path without a user-specific directory. These binaries are x86_64 and require Rosetta on Apple Silicon.
