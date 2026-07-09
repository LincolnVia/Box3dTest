#include "world.hpp"
#include "../Engine/engine.hpp"
#include "../graphics.hpp"
#include "cubeBrush.hpp"

#include "../rlights.h"
#include "../utils.hpp"
#include <box3d/box3d.h>
#include <box3d/math_functions.h>
#include <box3d/types.h>
#include <raylib.h>

enum Materials {

};

World::World(Engine &engine) : engine(engine) {
  b3WorldDef worldDef = b3DefaultWorldDef();
  worldDef.gravity = b3Vec3{0.0f, -10.0f, 0.0f};
  worldDef.enableContinuous = true;
  worldDef.contactHertz = 60.0f;
  worldDef.contactDampingRatio = 8.0f;
  worldDef.contactSpeed = 10.0f;
  worldDef.maximumLinearSpeed = 30.0f;
  worldId = b3CreateWorld(&worldDef);

  // Ground Plane
  sceneObjects.push_back(createStaticObject({0, -5, 0}, {50, 10, 50}, worldId));
  sceneObjects[0].name = "ground";

  // Speed Gel
  sceneObjects.push_back(createStaticObject({-5, 0.1, 0}, {3, .2, 20}, worldId,
                                            SurfaceMaterial::SpeedGel));
  // Speed Gel
  sceneObjects.push_back(createStaticObject({-5, 0.1, 15}, {3, .3, 10}, worldId,
                                            SurfaceMaterial::BounceGel));

  sceneObjects.push_back(createStaticObject({0, 0, 20}, {10, 10, 5}, worldId));
  sceneObjects[1].name = "portalable_wall";

  sceneObjects.push_back(createDynamicObject(
      {0, 2, 0}, {2, 2, 2}, CompanionCubeStats::density,
      CompanionCubeStats::friction, worldId, SurfaceMaterial::CompanionCube));
  sceneObjects[4].name = "companion_cube";

  sceneButtonsObjects.push_back(
      createPhysicsButton({5, 0.3, 0}, worldId, 1, 1));

  // END

  // Dynamic Box

  // End
}

void World::Init() {

  skyBox = createSkybox(engine.getShaderManager().getShader("cubemap"));
  CreateLight(LIGHT_DIRECTIONAL, {0, 0, 0}, {-1, 0, -1}, WHITE,
              engine.getShaderManager().getShader("lighting"));
  cubeBrush brush = {
      b3WorldTransform{b3ToPos({0.0f, -2.0f, 50.0f}), b3Quat_identity},
      b3Vec3{100.0f, 1.0f, 100.0f}, WHITE};

  meshs.push_back(brush);
}

World::~World() { b3DestroyWorld(worldId); }

void World::Update() {
  b3World_Step(worldId, timeStep, subStepCount);
  ApplyCompanionCubeReleasedTuning();
}

void World::ApplyCompanionCubeReleasedTuning() {
  for (auto &object : sceneObjects) {
    if (object.name != "companion_cube") {
      continue;
    }

    // Held cubes have gravity disabled by Player, so leave them fully under the
    // pickup joint. Ground drag only affects released cubes.
    if (b3Body_GetGravityScale(object.bodyId) <= 0.0f) {
      continue;
    }

    // Player movement is velocity-driven, so a released cube can gain too much
    // solver energy if it is crushed against a wall. Clamp the free cube every
    // step, not just when it is dropped.
    ClampBodyVelocity(object.bodyId, CompanionCubeStats::freeMaxSpeed,
                      CompanionCubeStats::freeMaxAngularSpeed);
    ResolveStaticPenetration(object.bodyId, object.shapeId,
                             CompanionCubeStats::staticPenetrationSlop,
                             CompanionCubeStats::staticDepenetrationBias);

    b3QueryFilter filter = b3DefaultQueryFilter();
    filter.maskBits = CollisionCategory::Static;

    const b3RayResult groundHit = b3World_CastRayClosest(
        worldId, b3Body_GetPosition(object.bodyId),
        {0.0f, -object.halfExtents.y - CompanionCubeStats::groundCheckDistance,
         0.0f},
        filter);

    if (!groundHit.hit) {
      continue;
    }

    const b3Vec3 velocity = b3Body_GetLinearVelocity(object.bodyId);
    const b3Vec3 horizontalVelocity = {velocity.x, 0.0f, velocity.z};
    ApplyVectorDrag(object.bodyId, horizontalVelocity,
                    CompanionCubeStats::groundLinearDrag,
                    CompanionCubeStats::groundStopSpeed, timeStep, true);

    ApplyVectorDrag(object.bodyId, b3Body_GetAngularVelocity(object.bodyId),
                    CompanionCubeStats::groundAngularDrag,
                    CompanionCubeStats::groundStopAngularSpeed, timeStep,
                    false);
  }
}
void World::DrawGameObjects() {
  for (auto &object : GetSceneObjects()) {
    if (object.material == SurfaceMaterial::Default)
      b3_DrawCubeTex(engine.getAssetManager().getTexture("ground"),
                     b3Body_GetPosition(object.bodyId), object.halfExtents,
                     WHITE);
    if (object.material == SurfaceMaterial::PortalWall)
      b3_DrawCubeTex(engine.getAssetManager().getTexture("wall"),
                     b3Body_GetPosition(object.bodyId), object.halfExtents,
                     WHITE);

    if (object.name == "companion_cube")
      b3_DrawModel(engine.getAssetManager().getModel("companionCube"),
                   B3WorldTransformToMatrix(b3Body_GetTransform(object.bodyId),
                                            object.halfExtents));
    if (object.material == SurfaceMaterial::SpeedGel)
      b3_DrawCubeTex(engine.getAssetManager().getTexture("speedGel"),
                     b3Body_GetPosition(object.bodyId), object.halfExtents,
                     WHITE);
    if (object.material == SurfaceMaterial::BounceGel)
      b3_DrawCubeTex(engine.getAssetManager().getTexture("bounceGel"),
                     b3Body_GetPosition(object.bodyId), object.halfExtents,
                     WHITE);
  }
  for (auto &object : GetSceneButtons()) {
    b3_DrawModel(
        engine.getAssetManager().getModel("storageCube"),
        B3WorldTransformToMatrix(
            b3Body_GetTransform(object.colliderInfo.bodyId), object.scale));
  }
}
void World::Draw() {
  DrawSkybox(skyBox);
  DrawGameObjects();
}
