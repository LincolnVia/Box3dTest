#pragma once

#include <box3d/box3d.h>
#include <box3d/math_functions.h>
#include <cstdint>
#include <string>

namespace CollisionCategory {
constexpr uint64_t Static = 1ull << 0;
constexpr uint64_t Dynamic = 1ull << 1;
constexpr uint64_t Player = 1ull << 2;
constexpr uint64_t World = Static | Dynamic;
constexpr uint64_t All = Static | Dynamic | Player;
} // namespace CollisionCategory

namespace CompanionCubeStats {
constexpr float friction = 0.5f;
constexpr float density = 1.0f;

} // namespace CompanionCubeStats

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