# Entity use cases

This guide describes how TrenchBroom entities move through this project: from
their FGD definitions, into the compiled BSP, and then into the game. Entity
properties in a BSP are strings. `EntityView` in `src/game/world/entityRegistry.*`
provides typed access, while `EntityRegistry` dispatches runtime entities by
their `classname`.

## Quick map of the paths

| Entity use case | Example classname | Runtime path |
| --- | --- | --- |
| Player start | `info_player_start`, `info_player_deathmatch` | Read directly by the BSP loader |
| Static model with no dynamic body | `prop_static` | Created while the BSP loads |
| Model with a dynamic Box3D body | `prop_dynamic` | Dispatched by `World` registry; parsed by `ParseModelEntity` |
| Weapon or wallbuy model | `weapon_1911`, `weapon_ak47`, `weapon_remmington`, `wallbuy` | Dispatched by `World` registry; parsed by `ParseModelEntity` |
| Brush trigger | `info_trigger` | Dispatched by `World`; BSP brush model `*N` supplies its bounds |
| Zombie spawner data | `info_zombie_spawn` | Read by `ZombieManager::ReadLevelData` using `EntityView` |
| Lighting | `light`, `triggerd_light` | Used by TrenchBroom and the BSP lighting compiler; not spawned by `World` |
| Custom gameplay entity | Your classname | Add an FGD definition, register a handler, and implement its behavior |

## 1. Player starts

Place `info_player_start` or `info_player_deathmatch` in the map. The BSP loader
reads `origin` and `angle` directly in `FindPlayerSpawn`; these classes do not
go through `EntityRegistry`. The selected position and yaw are exposed by
`World::GetBspPlayerSpawn()` and `World::HasBspPlayerSpawn()`.

Use these entities when the game should place the player at a map-defined
location. Their FGD definitions are in `tools/trenchbroom/VEngine/VEngine.fgd`.

## 2. Static model props

Use `prop_static` for a model loaded as part of the BSP level. Its `model` value
is relative to `resources/models`; `origin`, `angle`, and `scale` set placement,
yaw in degrees, and uniform size. For example:

```text
"classname" "prop_static"
"model" "brick_fence/brick_wall.glb"
"origin" "0 0 0"
"angle" "90"
"scale" "1"
```

Static props are found in `FindStaticProps` in `src/game/world/quakeBsp.cpp`, not in
the runtime registry. Use this path for scenery that does not need a Box3D body
or per-frame game behavior.

## 3. Dynamic model props

Use `prop_dynamic` for a model that needs a Box3D dynamic body. The class is
registered in `World::RegisterEntitySpawners`. Its handler calls
`ParseModelEntity`, which validates the model path and transform, loads the
model, and calculates scaled bounds. The handler creates the Box3D object and
stores it in `dynamicProps` for drawing and simulation.

The shared model parser expects a model and origin. `angle` defaults to zero
and `scale` defaults to one. A supplied scale must be a positive number.

## 4. Wallbuy and weapon model props

`wallbuy`, `weapon_1911`, `weapon_ak47`, and `weapon_remmington` all use the
same handler and shared model parser. The weapon classnames are the concrete
classes exposed by the FGD; registering each classname lets them share the
wallbuy implementation.

The handler creates a model-backed Box3D object and reads `cost` and
`purchased`. Wallbuy models are drawn using their entity scale. To add another
FGD subclass of the wallbuy base class, register its classname in
`World::RegisterEntitySpawners` as well.

## 5. Brush triggers

Use `info_trigger` on a brush volume. QBSP writes the brush model as a key such
as `"model" "*1"`; the number identifies the BSP model whose bounds become the
trigger volume. `World::SpawnTrigger` resolves that model, creates the sensor,
and reads `target`, `triggered`, and `enabled` through `EntityView`.

This differs from a point entity: the volume comes from the BSP brush model,
not from the entity's point origin. Its classname is registered in
`World::RegisterEntitySpawners`.

## 6. Zombie spawner data

`info_zombie_spawn` is read in `ZombieManager::ReadLevelData`. It uses
`EntityView` for `origin`, `spawnrate`, `enabled`, and `limit`. Defaults are
spawn rate `1`, enabled `1`, and unlimited spawns (`-1`). Its data is collected
in `zombieSpawners`; it is not currently dispatched by `World::EntityRegistry`.

## 7. Lighting entities

`light` and `triggerd_light` are defined for the editor and BSP lighting
workflow. They are not registered as game runtime objects. Add a runtime
handler only if the game itself needs to create or change lights after loading
the BSP.

## Add a new registry entity

### A. Add the TrenchBroom definition

Add an FGD class with a clear classname and defaults in
`tools/trenchbroom/VEngine/VEngine.fgd`. For a data-only point entity:

```fgd
@PointClass size(-8 -8 -8, 8 8 8) color(100 255 100) = item_health : "Health pickup"
[
    amount(integer) : "Health amount" : 25
]
```

TrenchBroom writes the classname and edited properties to the compiled BSP.
Point entities also receive an `origin` from their placement in the editor.

### B. Register its classname

Add a registration in `World::RegisterEntitySpawners` in
`src/game/world/world.cpp`:

```cpp
entitySpawners.Register("item_health", [this](const Entity &entity) {
  SpawnHealthPickup(entity);
});
```

Declare `SpawnHealthPickup(const Entity &entity)` in `src/game/world/world.hpp` and
implement it in `world.cpp`. `SpawnEntities()` clears previously spawned
runtime objects, then dispatches all BSP entities to their registered
handlers.

### C. Read properties and create behavior

Wrap the map entity in `EntityView` and provide defaults for optional values:

```cpp
void World::SpawnHealthPickup(const Entity &entity) {
  const EntityView view(entity);
  const Vector3 position = view.Vector("origin", Vector3{});
  const int amount = view.Int("amount", 25);

  // Create the pickup at position and store amount for its behavior.
}
```

`EntityView` supports:

- `String(key, fallback)` for text.
- `Int(key, fallback)` and `Float(key, fallback)` for numbers.
- `Bool(key, fallback)` for integer-style boolean values.
- `Vector(key, fallback)` for Quake `origin` values, converted to game space.
- `TryInt`, `TryFloat`, and `TryVector` when malformed input should reject the
  entity; invalid values are logged.
- `ClassName()` to read its `classname`.

For model-backed handlers, use `ParseModelEntity(engine, view, className)` and
handle the returned `std::optional<ModelEntity>`. That helper centralizes
model path validation, transform parsing, asset loading, scale, and collision
bounds. Use the returned transform and size when creating the Box3D body.

For a new runtime type, add storage to `World` if it needs to persist. Include
it in `ClearSpawnedEntities` if it owns Box3D bodies, and in the update or draw
loops if it needs simulation or rendering. Keep brush entities on a brush
specific path like `info_trigger`, since their geometry is referenced through
the BSP model key.

## Compile and inspect

Compile the map with TrenchBroom's `Compile` profile, then load its BSP in the
game. Check that the classname was registered and that the required keys are
present. Invalid values read through `TryInt`, `TryFloat`, or `TryVector` are
reported in the game log; shared model parsing reports why an entity was
skipped.
