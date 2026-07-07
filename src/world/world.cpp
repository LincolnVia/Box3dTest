#include "world.hpp"
#include "../graphics.hpp"
#include "cubeBrush.hpp"

#include "../utils.hpp"
#include <box3d/box3d.h>
#include <box3d/math_functions.h>
#include <box3d/types.h>
#include <raylib.h>

enum Materials {

};

World::World() {
  b3WorldDef worldDef = b3DefaultWorldDef();
  worldDef.gravity = b3Vec3{0.0f, -10.0f, 0.0f};
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

void World::Update() { b3World_Step(worldId, timeStep, subStepCount); }

void World::Draw() {
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
