#pragma once

#include "physics.hpp"
#include <box3d/collision.h>
#include <box3d/math_functions.h>

namespace ButtonStats {
/// Radius of the button collider at floor height. Wider bottom makes a ramp.
constexpr float colliderBottomRadius = 1.65f;
/// Radius of the button collider at the top. Smaller top creates sloped sides.
constexpr float colliderTopRadius = 1.05f;
/// Walkable height of the button collider.
constexpr float colliderHeight = 0.35f;
/// Local vertical offset so the collider starts on the floor while the model
/// can keep its visual origin.
constexpr float colliderYOffset = -0.3f;
/// Number of hull slices. Higher is rounder but costs more collision faces.
constexpr int colliderSides = 16;
} // namespace ButtonStats

struct PhysicsButton {
  BoxDef colliderInfo;
  b3Vec3 scale = {3.0f, 3.0f, 3.0f};
  const int buttonId;
  const int doorId;
  bool activated = false;
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

  staticObject.halfExtents = {ButtonStats::colliderBottomRadius,
                              ButtonStats::colliderHeight * 0.5f,
                              ButtonStats::colliderBottomRadius};

  b3ShapeDef staticShapeDef = b3DefaultShapeDef();
  staticShapeDef.filter.categoryBits = CollisionCategory::Static;
  staticShapeDef.filter.maskBits =
      CollisionCategory::Dynamic | CollisionCategory::Player;

  b3HullData *buttonHull =
      b3CreateCone(ButtonStats::colliderHeight,
                   ButtonStats::colliderBottomRadius,
                   ButtonStats::colliderTopRadius, ButtonStats::colliderSides);

  const b3Transform colliderTransform = {
      {0.0f, ButtonStats::colliderYOffset, 0.0f},
      b3Quat_identity,
  };
  staticObject.shapeId = b3CreateTransformedHullShape(
      staticObject.bodyId, &staticShapeDef, buttonHull, colliderTransform,
      b3Vec3_one);
  b3DestroyHull(buttonHull);
  staticObject.name = "physicsButton";

  return PhysicsButton{staticObject, buttonid, doorid};
}
