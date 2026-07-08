#pragma once

#include "engine/global.hpp"
#include "graphics.hpp"
#include "world/world.hpp"
#include <box3d/box3d.h>
#include <box3d/id.h>
#include <box3d/math_functions.h>
#include <raylib.h>

struct RayFireResult {
  b3RayResult hit{};
  b3Pos origin{};
  b3Vec3 translation{};
};

inline float CastClosestRayResult(b3ShapeId shapeId, b3Pos point,
                                  b3Vec3 normal, float fraction,
                                  uint64_t materialId, int triangleIndex,
                                  int childIndex, void *context) {
  b3RayResult *result = static_cast<b3RayResult *>(context);
  result->shapeId = shapeId;
  result->point = point;
  result->normal = normal;
  result->userMaterialId = materialId;
  result->fraction = fraction;
  result->triangleIndex = triangleIndex;
  result->childIndex = childIndex;
  result->hit = true;

  return fraction;
}

inline RayFireResult FireRay(const b3Vec3 &position, const b3Vec3 direction,
                             const World &world, float distance,
                             uint64_t maskBits = B3_DEFAULT_MASK_BITS) {

  const b3Vec3 rayOrigin = position;
  const b3Vec3 rayDirection = b3Normalize(direction);

  RayFireResult result;
  result.origin = b3ToPos(rayOrigin);
  result.translation = b3MulSV(distance, rayDirection);

  b3QueryFilter filter = b3DefaultQueryFilter();
  filter.maskBits = maskBits;

  result.hit = b3World_CastRayClosest(world.GetWorldId(), result.origin,
                                      result.translation, filter);
  if (debugMode) {
    DrawLine3D(B3PosToVector3(result.origin),
               B3PosToVector3(b3OffsetPos(result.origin, result.translation)),
               GREEN);
  }
  return result;
}
inline RayFireResult FireCameraRay(const Camera3D &camera, const World &world,
                                   float distance,
                                   uint64_t maskBits = B3_DEFAULT_MASK_BITS) {
  const Ray screenRay = GetScreenToWorldRay(GetMousePosition(), camera);

  const b3Vec3 rayOrigin = Vector3ToB3Vec3(screenRay.position);
  const b3Vec3 rayDirection = b3Normalize(Vector3ToB3Vec3(screenRay.direction));

  RayFireResult result;
  result.origin = b3ToPos(rayOrigin);
  result.translation = b3MulSV(distance, rayDirection);

  b3QueryFilter filter = b3DefaultQueryFilter();
  filter.maskBits = maskBits;

  result.hit = b3World_CastRayClosest(world.GetWorldId(), result.origin,
                                      result.translation, filter);

  if (debugMode) {
    DrawLine3D(B3PosToVector3(result.origin),
               B3PosToVector3(b3OffsetPos(result.origin, result.translation)),
               GREEN);
  }

  return result;
}

inline RayFireResult FireCameraSphereRay(
    const Camera3D &camera, const World &world, float distance, float radius,
    uint64_t maskBits = B3_DEFAULT_MASK_BITS) {
  const Ray screenRay = GetScreenToWorldRay(GetMousePosition(), camera);

  const b3Vec3 rayOrigin = Vector3ToB3Vec3(screenRay.position);
  const b3Vec3 rayDirection = b3Normalize(Vector3ToB3Vec3(screenRay.direction));

  RayFireResult result;
  result.origin = b3ToPos(rayOrigin);
  result.translation = b3MulSV(distance, rayDirection);

  b3QueryFilter filter = b3DefaultQueryFilter();
  filter.maskBits = maskBits;

  const b3Vec3 sphereCenter = b3Vec3_zero;
  b3ShapeProxy proxy = {&sphereCenter, 1, radius};
  b3World_CastShape(world.GetWorldId(), result.origin, &proxy,
                    result.translation, filter, CastClosestRayResult,
                    &result.hit);

  if (debugMode) {
    const b3Pos end = b3OffsetPos(result.origin, result.translation);
    DrawLine3D(B3PosToVector3(result.origin), B3PosToVector3(end), GREEN);
    DrawSphereWires(B3PosToVector3(result.origin), radius, 12, 6, SKYBLUE);
    DrawSphereWires(B3PosToVector3(end), radius, 12, 6, SKYBLUE);
  }

  return result;
}
