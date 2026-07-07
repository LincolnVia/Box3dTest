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
  staticObjects.push_back(
      createStaticObject({0, -5, 0}, {50, 10, 50}, worldId));
  staticObjects.push_back(createStaticObject({20, 0, 0}, {3, 10, 10}, worldId));
  staticObjects.push_back(
      createStaticObject({-20, 0, 0}, {3, 10, 10}, worldId));
  staticObjects.push_back(
      createStaticObject({0, 0, -20}, {10, 10, 3}, worldId));
  staticObjects.push_back(createStaticObject({0, 0, 20}, {10, 10, 3}, worldId));
  staticObjects[0].name = "ground";

  // END

  // Dynamic Box
  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.type = b3_dynamicBody;
  bodyDef.position = b3ToPos({0.0f, 4.0f, 5.0f});
  companionCube.bodyId = b3CreateBody(worldId, &bodyDef);

  companionCube.halfExtents = {1.5f, 1.5f, 1.5f};
  companionCube.hull = b3MakeCubeHull(companionCube.halfExtents.x);

  b3ShapeDef shapeDef = b3DefaultShapeDef();
  shapeDef.density = 1.2f;
  shapeDef.baseMaterial.friction = 1.0f;
  shapeDef.filter.categoryBits = CollisionCategory::Dynamic;
  shapeDef.filter.maskBits = CollisionCategory::All;

  companionCube.shapeId = b3CreateHullShape(companionCube.bodyId, &shapeDef,
                                            &companionCube.hull.base);
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
  for (auto &object : staticObjects) {
    if (object.name == "ground")
      b3_DrawCubeTex(grassTex, b3Body_GetPosition(object.bodyId),
                     object.halfExtents, WHITE);
    else
      b3_DrawCubeTex(wallTex, b3Body_GetPosition(object.bodyId),
                     object.halfExtents, WHITE);
    ;
  }

  const b3WorldTransform t = b3Body_GetTransform(companionCube.bodyId);
  const b3Vec3 renderScale = {companionCube.halfExtents.x * 1.0f,
                              companionCube.halfExtents.y * 1.0f,
                              companionCube.halfExtents.z * 1.0f};

  b3_DrawModel(m_cube, B3WorldTransformToMatrix(t, renderScale));
  // DrawModel(m_world,{0,0,0},.05   ,WHITE);
  DrawBoxWireframe(t, companionCube.halfExtents, RED);
}
