#include "world.hpp"
#include "../Engine/engine.hpp"
#include "../graphics.hpp"
#include "cubeBrush.hpp"

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
  sceneObjects.push_back(createStaticObject({20, 0, 0}, {3, 10, 10}, worldId));
  sceneObjects.push_back(createStaticObject({-20, 0, 0}, {3, 10, 10}, worldId));
  sceneObjects.push_back(createStaticObject({0, 0, -20}, {10, 10, 3}, worldId));
  sceneObjects.push_back(createStaticObject({0, 0, 20}, {10, 10, 3}, worldId));
  sceneObjects[0].name = "ground";

  sceneObjects[1].name = "portalable_wall";
  sceneObjects[2].name = "portalable_wall";
  sceneObjects[3].name = "portalable_wall";
  sceneObjects[4].name = "portalable_wall";

  sceneObjects.push_back(
      createDynamicObject({0, 2, 0}, {2, 2, 2}, CompanionCubeStats::density,
                          CompanionCubeStats::friction, worldId));
  sceneObjects[5].name = "companion_cube";

  // END

  // Dynamic Box

  // End
}

void World::Init() {
  m_world = LoadModel("resources/scenes/level_01.obj");
  grassTex = LoadTexture("resources/assets/grass.png");
  wallTex = LoadTexture("resources/textures/Dark/texture_01.png");
  m_cube = LoadModel("resources/models/companion_cube.glb");
  skyBox = createSkybox(engine.shdrCubemap);
  cubeBrush brush = {
      b3WorldTransform{b3ToPos({0.0f, -2.0f, 50.0f}), b3Quat_identity},
      b3Vec3{100.0f, 1.0f, 100.0f}, WHITE};

  meshs.push_back(brush);
}

World::~World() {
  UnloadTexture(grassTex);
  UnloadModel(m_cube);
  b3DestroyWorld(worldId);
}

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

void World::Draw() {
  DrawSkybox(skyBox);
  for (auto &object : GetSceneObjects()) {
    if (object.name == "ground")
      b3_DrawCubeTex(grassTex, b3Body_GetPosition(object.bodyId),
                     object.halfExtents, WHITE);
    if (object.name == "portalable_wall")
      b3_DrawCubeTex(wallTex, b3Body_GetPosition(object.bodyId),
                     object.halfExtents, WHITE);

    if (object.name == "companion_cube")
      b3_DrawModel(m_cube,
                   B3WorldTransformToMatrix(b3Body_GetTransform(object.bodyId),
                                            object.halfExtents));
  }

  // const b3WorldTransform t = b3Body_GetTransform(companionCube.bodyId);
  // const b3Vec3 renderScale = {companionCube.halfExtents.x * 1.0f,
  //                            companionCube.halfExtents.y * 1.0f,
  //                            companionCube.halfExtents.z * 1.0f};

  // b3_DrawModel(m_cube, B3WorldTransformToMatrix(t, renderScale));
  //  DrawModel(m_world,{0,0,0},.05   ,WHITE);
  // DrawBoxWireframe(t, companionCube.halfExtents, RED);
}
