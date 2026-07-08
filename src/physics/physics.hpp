#pragma once

#include <box3d/box3d.h>
#include <box3d/math_functions.h>
#include <cstdint>
#include <string>
#include <vector>

namespace CollisionCategory {
constexpr uint64_t Static = 1ull << 0;
constexpr uint64_t Dynamic = 1ull << 1;
constexpr uint64_t Player = 1ull << 2;
constexpr uint64_t World = Static | Dynamic;
constexpr uint64_t All = Static | Dynamic | Player;
} // namespace CollisionCategory

namespace CompanionCubeStats {
/// Contact grip for the cube. Keep this near the normal 0..1 range; stopping
/// long floor slides is handled by ground drag instead of extreme friction.
constexpr float friction = 0.9f;
constexpr float density = 10.0f;
/// Extra distance below the cube used to decide if floor drag should run.
constexpr float groundCheckDistance = 0.2f;
/// Horizontal speed removed per second while the released cube is on the floor.
constexpr float groundLinearDrag = 14.0f;
/// Spin speed removed per second while the released cube is on the floor.
constexpr float groundAngularDrag = 20.0f;
/// Horizontal speed below this value snaps to zero to stop tiny floor skating.
constexpr float groundStopSpeed = 0.35f;
/// Spin speed below this value snaps to zero to stop slow endless tumbling.
constexpr float groundStopAngularSpeed = 0.8f;
/// Maximum linear speed for a released cube after any physics step. This caps
/// wall/player squeeze energy without making the cube harder to pick up.
constexpr float freeMaxSpeed = 8.0f;
/// Maximum spin speed for a released cube after any physics step.
constexpr float freeMaxAngularSpeed = 18.0f;
/// Allowed overlap with static walls before the cube is nudged back out.
constexpr float staticPenetrationSlop = 0.03f;
/// Extra distance added to static depenetration so the cube clears the wall.
constexpr float staticDepenetrationBias = 0.01f;

} // namespace CompanionCubeStats

/// Clamp a vector to a maximum length while keeping its direction.
inline b3Vec3 ClampVectorLength(b3Vec3 vector, float maxLength) {
  const float length = b3Length(vector);

  if (maxLength <= 0.0f || length <= maxLength || length <= 0.0f) {
    return vector;
  }

  return b3MulSV(maxLength / length, vector);
}

/// Clamp a body's linear and angular velocity without changing direction.
inline void ClampBodyVelocity(b3BodyId bodyId, float maxSpeed,
                              float maxAngularSpeed) {
  b3Body_SetLinearVelocity(
      bodyId, ClampVectorLength(b3Body_GetLinearVelocity(bodyId), maxSpeed));
  b3Body_SetAngularVelocity(
      bodyId,
      ClampVectorLength(b3Body_GetAngularVelocity(bodyId), maxAngularSpeed));
}

/// Push a body out of static geometry and remove velocity into the wall.
inline void ResolveStaticPenetration(b3BodyId bodyId, b3ShapeId shapeId,
                                     float slop, float bias) {
  const int capacity = b3Shape_GetContactCapacity(shapeId);
  if (capacity <= 0) {
    return;
  }

  std::vector<b3ContactData> contacts(capacity);
  const int count = b3Shape_GetContactData(shapeId, contacts.data(), capacity);

  b3Vec3 correction = b3Vec3_zero;
  b3Vec3 velocity = b3Body_GetLinearVelocity(bodyId);

  for (int contactIndex = 0; contactIndex < count; ++contactIndex) {
    const b3ContactData &contact = contacts[contactIndex];
    const bool cubeIsShapeA = B3_ID_EQUALS(contact.shapeIdA, shapeId);
    const b3ShapeId otherShapeId =
        cubeIsShapeA ? contact.shapeIdB : contact.shapeIdA;
    const b3Filter otherFilter = b3Shape_GetFilter(otherShapeId);

    if ((otherFilter.categoryBits & CollisionCategory::Static) == 0) {
      continue;
    }

    for (int manifoldIndex = 0; manifoldIndex < contact.manifoldCount;
         ++manifoldIndex) {
      const b3Manifold &manifold = contact.manifolds[manifoldIndex];
      const b3Vec3 outNormal =
          cubeIsShapeA ? b3Neg(manifold.normal) : manifold.normal;
      float deepestPenetration = 0.0f;

      for (int pointIndex = 0; pointIndex < manifold.pointCount; ++pointIndex) {
        deepestPenetration = b3MaxFloat(
            deepestPenetration, -manifold.points[pointIndex].separation);
      }

      if (deepestPenetration <= slop) {
        continue;
      }

      correction =
          b3MulAdd(correction, deepestPenetration - slop + bias, outNormal);

      const float wallSpeed = b3Dot(velocity, outNormal);
      if (wallSpeed < 0.0f) {
        velocity = b3MulSub(velocity, wallSpeed, outNormal);
      }
    }
  }

  if (b3LengthSquared(correction) <= 0.0f) {
    return;
  }

  const b3WorldTransform transform = b3Body_GetTransform(bodyId);
  const b3Vec3 position = b3Add(b3ToVec3(transform.p), correction);
  b3Body_SetTransform(bodyId, b3ToPos(position), transform.q);
  b3Body_SetLinearVelocity(bodyId, velocity);
}

/// Remove speed from either a body's horizontal velocity or angular velocity.
inline void ApplyVectorDrag(b3BodyId bodyId, b3Vec3 velocity, float drag,
                            float stopSpeed, float dt, bool linear) {
  const float speed = b3Length(velocity);

  if (speed <= 0.0f) {
    return;
  }

  float newSpeed = b3MaxFloat(0.0f, speed - drag * dt);
  if (newSpeed <= stopSpeed) {
    newSpeed = 0.0f;
  }

  const b3Vec3 dampedVelocity = b3MulSV(newSpeed / speed, velocity);

  if (linear) {
    b3Vec3 bodyVelocity = b3Body_GetLinearVelocity(bodyId);
    bodyVelocity.x = dampedVelocity.x;
    bodyVelocity.z = dampedVelocity.z;
    b3Body_SetLinearVelocity(bodyId, bodyVelocity);
  } else {
    b3Body_SetAngularVelocity(bodyId, dampedVelocity);
  }
}

struct BoxDef {
  std::string name;
  b3BodyId bodyId{};
  b3ShapeId shapeId{};
  b3BoxHull hull{};
  b3Vec3 halfExtents{};
};
/**
 * @brief Create a Static Object object
 *
 * @param position
 * @param scale
 * @param worldID
 * @return BoxDef
 */

inline BoxDef createStaticObject(b3Vec3 position, b3Vec3 scale,
                                 b3WorldId worldID) {
  BoxDef staticObject{};
  b3BodyDef staticBodyDef = b3DefaultBodyDef();
  staticBodyDef.position = b3ToPos(position);
  staticBodyDef.type = b3_staticBody;
  staticObject.bodyId = b3CreateBody(worldID, &staticBodyDef);

  staticObject.halfExtents = {scale.x / 2, scale.y / 2, scale.z / 2};
  staticObject.hull =
      b3MakeBoxHull(staticObject.halfExtents.x, staticObject.halfExtents.y,
                    staticObject.halfExtents.z);

  b3ShapeDef staticShapeDef = b3DefaultShapeDef();
  staticShapeDef.filter.categoryBits = CollisionCategory::Static;
  staticShapeDef.filter.maskBits =
      CollisionCategory::Dynamic | CollisionCategory::Player;
  staticObject.shapeId = b3CreateHullShape(staticObject.bodyId, &staticShapeDef,
                                           &staticObject.hull.base);
  return staticObject;
}
/**
 * @brief Create a Dynamic Object object
 *
 * @param position
 * @param scale
 * @param density
 * @param friction
 * @param worldID
 * @return BoxDef
 */

inline BoxDef createDynamicObject(b3Vec3 position, b3Vec3 scale, float density,
                                  float friction, b3WorldId worldID) {
  BoxDef dynamicObject{};

  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.type = b3_dynamicBody;
  bodyDef.position = b3ToPos(position);
  bodyDef.isBullet = true;
  dynamicObject.bodyId = b3CreateBody(worldID, &bodyDef);

  dynamicObject.halfExtents = {scale.x / 2, scale.y / 2, scale.z / 2};
  dynamicObject.hull =
      b3MakeBoxHull(dynamicObject.halfExtents.x, dynamicObject.halfExtents.y,
                    dynamicObject.halfExtents.z);

  b3ShapeDef shapeDef = b3DefaultShapeDef();
  shapeDef.density = density;
  shapeDef.baseMaterial.friction = friction;
  shapeDef.filter.categoryBits = CollisionCategory::Dynamic;
  shapeDef.filter.maskBits = CollisionCategory::All;

  dynamicObject.shapeId = b3CreateHullShape(dynamicObject.bodyId, &shapeDef,
                                            &dynamicObject.hull.base);

  return dynamicObject;
};
