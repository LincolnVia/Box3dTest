# Box3dTest

A small first-person 3D sandbox built with C++20, Box3D, and raylib. The game
can render Quake BSP29 maps, build static collision meshes from their world
geometry, load loose textures, and use BSP player-start entities.

## Requirements

- CMake 4.2 or newer
- A Make-compatible build tool
- MinGW-w64 (`x86_64-w64-mingw32-gcc`, `g++`, and `windres`)
- Internet access during the first configuration to fetch dependencies

## Build

Windows is the default target:

```sh
./build.sh --game
```

Other targets:

```sh
./build.sh --editor
./build.sh --all
```

Executables are written to `build/windows-dev/`.

## Run

```sh
./build/windows-dev/Box3dGame.exe
```

The game loads `resources/scenes/initial_box3d.bsp` by default. To load a
different BSP29 map, pass it on the command line or drag it onto the running
window:

```sh
./build/windows-dev/Box3dGame.exe path/to/map.bsp
```

Runtime resources are copied beside the executable after each build.

## Controls

- `W`, `A`, `S`, `D`: move
- Mouse: look
- `Space`: jump
- `E`: pick up or drop the companion cube
- Left mouse button: fire the interaction ray

## BSP maps and textures

- Supported map format: Quake BSP version 29.
- Put loose textures in `resources/textures/` as PNG, JPG, or JPEG files.
- Texture filenames should match the material names used by the map.
- The loader accounts for Quake's 15-character material-name limit and uses
  the loose image dimensions when calculating UV coordinates.
- An `info_player_start` or `info_player_deathmatch` entity sets the spawn.

## TrenchBroom

The game definition and example profiles are in `tools/trenchbroom/`.

1. Configure TrenchBroom with `tools/trenchbroom/Box3dTest`.
2. Export the map and compile it with QBSP, VIS, and LIGHT.
3. Launch `Box3dGame.exe` with the generated BSP path.

See [tools/trenchbroom/README.md](tools/trenchbroom/README.md) and
[tools/trenchbroom/PROFILE_SETUP.md](tools/trenchbroom/PROFILE_SETUP.md) for
the included profile details. Their paths are machine-specific and may need to
be adjusted.

## Project layout

```text
src/                 Engine, game, editor, player, and world code
resources/           Runtime models, scenes, shaders, and textures
tools/trenchbroom/   TrenchBroom game configuration and map tools
cmake/               Toolchain and compatibility files
```
