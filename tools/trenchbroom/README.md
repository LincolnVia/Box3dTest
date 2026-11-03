# TrenchBroom

Set the TrenchBroom game path to the project root (the directory containing `resources/` and `tools/`).
Use the `Box3dTest` game configuration from `tools/trenchbroom/Box3dTest/GameConfig.cfg`.

Run `Build BSP and Run Box3D` to compile the map and launch its `.bsp`.
The generated `.bsp` is beside the source `.map`; the viewer also accepts drag-and-drop BSP files.

Set the `qbsp`, `vis`, and `light` compilation tools in TrenchBroom's game preferences. The compile profile refers to them through the configured tool variables.
