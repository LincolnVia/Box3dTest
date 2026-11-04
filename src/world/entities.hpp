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
struct prop_dynamic {
  BoxDef boxdef;
  float modelScale = 1.0f;
  int enabled = 1;
  prop_dynamic(BoxDef object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
};

struct info_trigger {
  b3ShapeId shapeid;
  int target = -1;
  int triggered = 0;
  int enabled = 1;
  info_trigger(Vector3 center, Vector3 halfExtents, b3WorldId &worldId) {
    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.position = {center.x, center.y, center.z};
    bodyDef.type = b3BodyType::b3_staticBody;

    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.filter.categoryBits = CollisionCategory::Trigger;
    shapeDef.isSensor = true;
    shapeDef.enableSensorEvents = true;
    b3BoxHull hull = b3MakeBoxHull(halfExtents.x, halfExtents.y, halfExtents.z);
    b3BodyId bodyId = b3CreateBody(worldId, &bodyDef);
    shapeid = b3CreateHullShape(bodyId, &shapeDef, &hull.base);
  }
  void on_trigger_collided() {
    if (enabled)
      triggered = 1;
  }
  void Update() {
    // First determine the required array capacity to hold all the overlapping
    // shape ids.
    int capacity = b3Shape_GetSensorCapacity(shapeid);
    std::vector<b3ShapeId> overlaps(static_cast<size_t>(capacity));

    // Now get all overlaps and record the actual count
    int count = b3Shape_GetSensorData(shapeid, overlaps.data(), capacity);

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
