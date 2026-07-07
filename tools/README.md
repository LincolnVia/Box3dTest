# TrenchBroom Map Converter

`MapToJsonConverter` converts a TrenchBroom / Quake `.map` file into this project's JSON scene format.

Build only the converter:

```powershell
cmake --build build --target MapToJsonConverter --config Debug
```

Convert `tools/level_01.map`:

```powershell
.\build\Debug\MapToJsonConverter.exe tools\level_01.map resources\scenes\level_01.json
```

The converter reads:

- entities and key/value properties
- brush plane definitions
- brush textures/material names
- exact convex brush bounds computed from plane intersections
- point entity `origin` values

By default, it converts TrenchBroom / Quake coordinates from Z-up `[x, y, z]` to the engine's Y-up `[x, z, y]`.

Useful options:

```powershell
.\build\Debug\MapToJsonConverter.exe tools\level_01.map resources\scenes\level_01.json --scale 0.03125
.\build\Debug\MapToJsonConverter.exe tools\level_01.map resources\scenes\level_01.json --raw-coordinates
.\build\Debug\MapToJsonConverter.exe tools\level_01.map resources\scenes\level_01.json --portalable-materials white_panel,portal_wall
```

`--portalable-materials` marks any brush using one of those texture names as `IsPortalable: true`. Textures containing `portalable` are also treated as portalable.
