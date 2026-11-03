#include "world.hpp"
#include "../Engine/engine.hpp"
#include "../graphics.hpp"
#include "cubeBrush.hpp"

#include "../rlights.h"
#include "../utils.hpp"
#include "entities.hpp"
#include <box3d/box3d.h>
#include <box3d/math_functions.h>
#include <box3d/types.h>
#include <raylib.h>
#include <string>
#include <vector>

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

void World::Shutdown() {
  if (isShutdown) {
    return;
  }
  quakeLevel.Unload();
  b3DestroyWorld(worldId);
  worldId = b3_nullWorldId;
  isShutdown = true;
}

World::~World() { Shutdown(); }

bool World::LoadBspLevel(const std::filesystem::path &path) {
  std::string error;
  if (!quakeLevel.Load(path, worldId, error)) {
    bspStatus = "BSP load failed: " + error;
    TraceLog(LOG_ERROR, "%s", bspStatus.c_str());
    return false;
  }

  bspStatus =
      TextFormat("%s (%d collision triangles)", quakeLevel.GetPath().c_str(),
                 quakeLevel.GetTriangleCount());
  for (BoxDef &object : sceneObjects) {
    b3Body_Disable(object.bodyId);
  }
  TraceLog(LOG_INFO, "Loaded BSP level: %s", bspStatus.c_str());
  return true;
}

void World::Update() {
  if (IsFileDropped()) {
    FilePathList droppedFiles = LoadDroppedFiles();
    if (droppedFiles.count > 0) {
      const std::filesystem::path path(droppedFiles.paths[0]);
      std::string extension = path.extension().string();
      std::transform(extension.begin(), extension.end(), extension.begin(),
                     [](unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                     });
      if (extension == ".bsp") {
        LoadBspLevel(path);
      } else if (extension != ".bsp") {
        TraceLog(LOG_WARNING, "Dropped file is not a .bsp: %s",
                 path.string().c_str());
      }
    }
    UnloadDroppedFiles(droppedFiles);
  }

  b3World_Step(worldId, timeStep, subStepCount);
}
void World::DrawGameObjects() {
  if (quakeLevel.IsLoaded()) {
    return;
  }
  for (auto &object : GetSceneObjects()) {

    if (object.type == ModelType::BRUSH)

    {
      b3_DrawCubeTex(engine.getAssetManager().getTexture(object.name),
                     b3Body_GetPosition(object.bodyId), object.halfExtents,
                     WHITE);
    }
    if (object.type == ModelType::MODEL) {
      b3_DrawModel(engine.getAssetManager().getModel(object.name),
                   B3WorldTransformToMatrix(b3Body_GetTransform(object.bodyId),
                                            object.halfExtents));
    }
  }
}
void World::Draw() {
  DrawSkybox(skyBox);
  quakeLevel.Draw(engine.getShaderManager().getShader("lighting"));
  DrawGameObjects();
}

void World::SpawnEntites() {
  std::vector<Entity> entities = quakeLevel.getEntities();
  triggers = SpawnTriggers();
}
std::vector<info_trigger> World::SpawnTriggers() {

  std::vector<info_trigger> triggers;
  for (const Entity &entity : quakeLevel.getEntities()) {
    const auto className = entity.find("classname");
    if (className == entity.end() || (className->second != "info_trigger")) {
      continue;
    }
    info_trigger trigger;
    if (auto it = entity.find("target"); it != entity.end()) {
      trigger.target = std::stoi(it->second);
    }
    if (auto it = entity.find("triggered"); it != entity.end()) {
      trigger.target = std::stoi(it->second);
    }
    if (auto it = entity.find("target"); it != entity.end()) {
      trigger.target = stoi(it->second);
    }

    triggers.push_back(trigger);
  }
  return triggers;
}
