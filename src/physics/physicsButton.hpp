#pragma once

#include "physics.hpp"
#include <box3d/math_functions.h>

struct PhysicsButton {
  BoxDef colliderInfo;
  b3Vec3 scale = {3, 3, 3};
  const int buttonId;
  const int doorId;
  bool activated;
  PhysicsButton(BoxDef colliderInfo, int buttonid, int doorid)
      : colliderInfo(colliderInfo), buttonId(buttonid), doorId(doorid) {}
};

inline PhysicsButton createPhysicsButton(b3Vec3 position, b3WorldId worldID,
                                         int buttonid, int doorid) {
  BoxDef staticObject{};
  b3BodyDef staticBodyDef = b3DefaultBodyDef();
  staticBodyDef.position = b3ToPos(position);
  staticBodyDef.type = b3_staticBody;
  staticObject.bodyId = b3CreateBody(worldID, &staticBodyDef);

  staticObject.halfExtents = {3, 1, 3};
  staticObject.hull =
      b3MakeBoxHull(staticObject.halfExtents.x, staticObject.halfExtents.y,
                    staticObject.halfExtents.z);

  b3ShapeDef staticShapeDef = b3DefaultShapeDef();
  staticShapeDef.filter.categoryBits = CollisionCategory::Static;
  staticShapeDef.filter.maskBits =
      CollisionCategory::Dynamic | CollisionCategory::Player;
  staticObject.shapeId = b3CreateHullShape(staticObject.bodyId, &staticShapeDef,
                                           &staticObject.hull.base);
  staticObject.name = "physicsButton";

  return *new PhysicsButton{staticObject, buttonid, doorid};
}