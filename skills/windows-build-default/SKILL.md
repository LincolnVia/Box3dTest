---
name: windows-build-default
description: Make Windows the default build target when creating or changing build configuration for coding projects. Use for build scripts, project setup, CMake presets, CI builds, and compile verification unless the user explicitly requests another platform.
---

# Windows build default

Treat a Windows artifact as the primary successful build outcome. Preserve an existing non-Windows build path when practical, but expose it as an explicit alternative rather than the default.

Choose the Windows toolchain that fits the environment and project:

- In WSL or Linux with MinGW-w64 available, prefer a reproducible cross-compilation toolchain and place output in a clearly Windows-specific build directory.
- On Windows with Visual Studio tooling, use an appropriate Visual Studio or Ninja generator.
- For managed or interpreted projects, use the ecosystem's Windows runtime identifier, packaging target, or equivalent artifact setting.

Verify the output format or target platform, not merely that compilation ran. For native code, inspect the resulting executable when possible. Do not claim success when only a host-native build passed.

Do not install toolchains, publish artifacts, or discard other platform configuration without the user's authorization. If Windows targeting is genuinely unsupported by a dependency, report the incompatibility and keep the project configuration usable.
