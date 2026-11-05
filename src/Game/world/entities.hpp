#pragma once

#include "../../engine/global.hpp"
#include "../global.hpp"
#include "../physics/physics.hpp"
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "box3d/types.h"
#include "raylib.h"
#include <cstddef>
#include <utility>
#include <vector>

namespace game {
enum EntityType { ZOMBIE, SKELETON };

class RuntimeEntity {
public:
  RuntimeEntity() = default;
  RuntimeEntity(const RuntimeEntity &) = delete;
  RuntimeEntity &operator=(const RuntimeEntity &) = delete;
  virtual ~RuntimeEntity() = default;
  virtual void Update() {}
  virtual void Interact() {}
};

inline void DestroyRuntimeBody(BoxDef &object) {
  if (B3_IS_NON_NULL(object.bodyId)) {
    b3DestroyBody(object.bodyId);
  }
}

struct EntitySpawner {
  Vector3 position;
  EntityType type;
};

struct wallbuy : RuntimeEntity {
  BoxDef boxdef;
  float modelScale = 1.0f;
  int cost = 0;
  int purchased = 0;
  int enabled = 1;
  wallbuy(BoxDef object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
  ~wallbuy() override { DestroyRuntimeBody(boxdef); }
};
struct prop_dynamic : RuntimeEntity {
  BoxDef boxdef;
  float modelScale = 1.0f;
  int enabled = 1;
  prop_dynamic(BoxDef object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
  ~prop_dynamic() override { DestroyRuntimeBody(boxdef); }
};

struct info_trigger : RuntimeEntity {
  BoxDef boxdef;
  int target = -1;
  int triggered = 0;
  int enabled = 1;
  info_trigger(Vector3 center, Vector3 halfExtents, b3WorldId &worldId) {
    boxdef = createStaticObject(
        {center.x, center.y, center.z},
        {halfExtents.x * 2.0f, halfExtents.y * 2.0f, halfExtents.z * 2.0f},
        worldId, SurfaceMaterial::Default, true);
  }
  ~info_trigger() override { DestroyRuntimeBody(boxdef); }
  void on_trigger_collided() {
    if (enabled)
      triggered = 1;
  }
  void Interact() override { on_trigger_collided(); }
  void Update() override {
    // First determine the required array capacity to hold all the overlapping
    // shape ids.
    int capacity = b3Shape_GetSensorCapacity(boxdef.shapeId);
    std::vector<b3ShapeId> overlaps(static_cast<size_t>(capacity));

    // Now get all overlaps and record the actual count
    int count =
        b3Shape_GetSensorData(boxdef.shapeId, overlaps.data(), capacity);

    for (int i = 0; i < count; ++i) {
      b3ShapeId visitorId = overlaps[static_cast<size_t>(i)];

      // Ensure the visitorId is valid
      if (b3Shape_IsValid(visitorId) == false) {
        continue;
      }

      // process overlap using game logic
      if (B3_IS_NULL(globalPlayerShapeId))
        return;

      if (B3_ID_EQUALS(visitorId, globalPlayerShapeId)) {
        if (!triggered) {
          triggered = true;
          TraceLog(LOG_INFO, "Trigger fired (target %d)", target);
        }
      }
    }
  }
};
struct info_zombie_spawn {
  info_zombie_spawn(b3Vec3 _pos) : position(_pos) {};
  b3Vec3 position;
  int spawnrate = 1; // Spawn Zombie Rate
  int limit = -1;    // Max Spawn Ammount | Special Case
  int enabled = 1;   // Should Update
  int spawned = 0;   // Spawn Count Tracker
};

struct info_reciever {
  int recieverID = -1;
  int triggered = 0;
};

} // namespace game
