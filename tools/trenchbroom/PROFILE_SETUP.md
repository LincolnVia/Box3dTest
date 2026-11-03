# TrenchBroom Compile Profile

The `Build BSP` profile is configured in `Box3dTest/CompilationProfiles.cfg`.
It exports a TrenchBroom-clean map, then runs QBSP, VIS, and LIGHT in order.
Compiler output is written alongside the source map.

The profile uses the ericw-tools 2.0.0-alpha11 macOS binaries installed at:

`/Users/lincoln/.local/opt/ericw-tools/`

The archive and bundled runtime libraries are kept together there so the tools
can load their dependencies. This build is x86_64 and runs on this Apple Silicon
Mac through Rosetta.
