#pragma once

#include <box3d/box3d.h>
#include <box3d/id.h>
#include <box3d/math_functions.h>
#include <raylib.h>
#include "graphics.hpp"
#include "physics.hpp"
#include "world/world.hpp"

struct RayFireResult
{
    b3RayResult hit{};
    b3Pos origin{};
    b3Vec3 translation{};
};

inline RayFireResult FireRay(
    const b3Vec3& position,
    const b3Vec3 direction,
    const World& world,
    float distance,
    uint64_t maskBits = B3_DEFAULT_MASK_BITS)
{
    

    const b3Vec3 rayOrigin = position;
    const b3Vec3 rayDirection = b3Normalize(direction);

    RayFireResult result;
    result.origin = b3ToPos(rayOrigin);
    result.translation = b3MulSV(distance, rayDirection);

    b3QueryFilter filter = b3DefaultQueryFilter();
    filter.maskBits = maskBits;

    result.hit = b3World_CastRayClosest(
        world.GetWorldId(),
        result.origin,
        result.translation,
        filter
    );

    return result;
}
inline RayFireResult FireCameraRay(
    const Camera3D& camera,
    const World& world,
    float distance,
    uint64_t maskBits = B3_DEFAULT_MASK_BITS)
{
    const Ray screenRay = GetScreenToWorldRay(GetMousePosition(), camera);

    const b3Vec3 rayOrigin = Vector3ToB3Vec3(screenRay.position);
    const b3Vec3 rayDirection = b3Normalize(Vector3ToB3Vec3(screenRay.direction));

    RayFireResult result;
    result.origin = b3ToPos(rayOrigin);
    result.translation = b3MulSV(distance, rayDirection);

    b3QueryFilter filter = b3DefaultQueryFilter();
    filter.maskBits = maskBits;

    result.hit = b3World_CastRayClosest(
        world.GetWorldId(),
        result.origin,
        result.translation,
        filter
    );

    return result;
}
inline BoxDef createStaticObject(b3Vec3 position,b3Vec3 scale,b3WorldId worldID)
{
    BoxDef staticObject{};
    b3BodyDef staticBodyDef = b3DefaultBodyDef();
    staticBodyDef.position = b3ToPos(position);
    staticBodyDef.type = b3_staticBody;
    staticObject.bodyId = b3CreateBody(worldID, &staticBodyDef);


    staticObject.halfExtents = { scale.x /2, scale.y /2 , scale.z/2 };
    staticObject.hull = b3MakeBoxHull(
        staticObject.halfExtents.x,
        staticObject.halfExtents.y,
        staticObject.halfExtents.z
    );

    b3ShapeDef staticShapeDef = b3DefaultShapeDef();
    staticShapeDef.filter.categoryBits = CollisionCategory::Static;
    staticShapeDef.filter.maskBits = CollisionCategory::Dynamic | CollisionCategory::Player;
    staticObject.shapeId = b3CreateHullShape(staticObject.bodyId, &staticShapeDef, &staticObject.hull.base);
    return staticObject;
}
