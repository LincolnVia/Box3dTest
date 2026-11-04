#pragma once

#include "../Engine/global.hpp"
#include "../physics/physics.hpp"
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/id.h"
#include "box3d/types.h"
#include "raylib.h"
#include <cstddef>
#include <utility>
#include <vector>
enum EntityType { ZOMBIE, SKELETON };

struct EntitySpawner {
  Vector3 position;
  EntityType type;
};

struct wallbuy {
  BoxDef boxdef;
  int cost = 0;
  int purchaced = 0;
  int enabled = 1;
  wallbuy(BoxDef object) : boxdef(std::move(object)) {}
};
struct prop_dynamic {
  BoxDef boxdef;
  float modelScale = 1.0f;
  int enabled = 1;
  prop_dynamic(BoxDef object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
};

struct info_trigger {
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
  void on_trigger_collided() {
    if (enabled)
      triggered = 1;
  }
  void Update() {
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

struct info_reciever {
  int recieverID = -1;
  int triggered = 0;
};
