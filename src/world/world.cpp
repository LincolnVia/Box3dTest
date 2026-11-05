#include "world.hpp"
#include "../Engine/engine.hpp"
#include "../graphics.hpp"
#include "cubeBrush.hpp"

#include "../rlights.h"
#include "../utils.hpp"
#include "entities.hpp"
#include "quakeUtil.hpp"
#include "raymath.h"
#include <algorithm>
#include <box3d/box3d.h>
#include <box3d/math_functions.h>
#include <box3d/types.h>
#include <cctype>
#include <cmath>
#include <raylib.h>
#include <string>
#include <utility>
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
  SpawnEntities();
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
  for (auto &trigger : triggers) {
    trigger.Update();
  }
}
void World::DrawGameObjects() {
  /*
   if (!quakeLevel.IsLoaded()) {
   for (auto &object : GetSceneObjects()) {
     if (object.type == ModelType::BRUSH) {
       b3_DrawCubeTex(engine.getAssetManager().getTexture(object.name),
                      b3Body_GetPosition(object.bodyId), object.halfExtents,
                      WHITE);
     }
     if (object.type == ModelType::MODEL) {
       b3_DrawModel(
           engine.getAssetManager().getModel(object.name),
           B3WorldTransformToMatrix(b3Body_GetTransform(object.bodyId),
                                    object.halfExtents));
     }
   }
 }
   */
  for (const prop_dynamic &prop : dynamicProps) {
    const b3Vec3 modelScale{prop.modelScale, prop.modelScale, prop.modelScale};
    b3_DrawModel(engine.getAssetManager().getModel(prop.boxdef.modelPath),
                 B3WorldTransformToMatrix(
                     b3Body_GetTransform(prop.boxdef.bodyId), modelScale));
  }
}
void World::Draw() {
  DrawSkybox(skyBox);
  quakeLevel.Draw(engine.getShaderManager().getShader("lighting"));
  DrawGameObjects();
}

void World::SpawnEntities() {
  triggers = SpawnTriggers();

  std::vector<prop_dynamic> spawnedProps = SpawnDynamicObjects();
  for (const prop_dynamic &prop : dynamicProps) {
    if (B3_IS_NON_NULL(prop.boxdef.bodyId)) {
      b3DestroyBody(prop.boxdef.bodyId);
    }
  }
  dynamicProps = std::move(spawnedProps);
}

std::vector<wallbuy> World::SpawnWallbuys() {

  for (const Entity &entity : quakeLevel.getEntities()) {
    const auto className = entity.find("classname");
    if (className == entity.end() || className->second != "wallbuy") {
      continue;
    }
    const auto model = entity.find("model");
    if (model == entity.end()) {
      TraceLog(LOG_WARNING, "Skipping wallbuy without a model");
      continue;
    }

    const std::filesystem::path modelPath = ResolveBspModelPath(model->second);
    if (modelPath.empty()) {
      TraceLog(LOG_WARNING, "Skipping prop_dynamic with invalid model: %s",
               model->second.c_str());
      continue;
    }

    const auto origin = entity.find("origin");
    Vector3 position{};
    if (origin == entity.end() || !ParseVector3(origin->second, position)) {
      TraceLog(LOG_WARNING, "Skipping prop_dynamic with invalid origin");
      continue;
    }

    const std::string modelKey = modelPath.generic_string();
    Model *loadedModel = engine.getAssetManager().LoadModelIfMissing(
        modelKey, modelPath.string());
    if (loadedModel == nullptr) {
      TraceLog(LOG_WARNING, "Could not load prop_dynamic model: %s",
               modelPath.string().c_str());
      continue;
    }

    float entityScale = 1.0f;
    if (const auto scale = entity.find("scale"); scale != entity.end()) {
      try {
        size_t parsedLength = 0;
        entityScale = std::stof(scale->second, &parsedLength);
        if (parsedLength != scale->second.size() ||
            !std::isfinite(entityScale) || entityScale <= 0.0f) {
          throw std::invalid_argument("invalid scale");
        }
      } catch (const std::exception &) {
        TraceLog(LOG_WARNING, "Skipping prop_dynamic with invalid scale: %s",
                 scale->second.c_str());
        continue;
      }
    }

    const BoundingBox bounds = GetModelBoundingBox(*loadedModel);
    const b3Vec3 fullSize{(bounds.max.x - bounds.min.x) * entityScale,
                          (bounds.max.y - bounds.min.y) * entityScale,
                          (bounds.max.z - bounds.min.z) * entityScale};
    if (fullSize.x <= 0.0f || fullSize.y <= 0.0f || fullSize.z <= 0.0f) {
      TraceLog(LOG_WARNING, "Skipping prop_dynamic with empty model bounds: %s",
               modelPath.string().c_str());
      continue;
    }

    const b3Vec3 physicsPosition{position.x, position.y, position.z};
    BoxDef object =
        createDynamicObject(physicsPosition, fullSize, 0.63f, 0.63f, worldId);
    object.type = ModelType::MODEL;
    object.modelPath = modelKey;

    if (const auto angle = entity.find("angle"); angle != entity.end()) {
      try {
        const float radians = std::stof(angle->second) * PI / 180.0f;
        const b3Quat rotation =
            b3MakeQuatFromAxisAngle({0.0f, 1.0f, 0.0f}, radians);
        b3Body_SetTransform(object.bodyId, b3ToPos(physicsPosition), rotation);
      } catch (const std::exception &) {
        TraceLog(LOG_WARNING, "Ignoring invalid wallbuy angle: %s",
                 angle->second.c_str());
      }
    }
    wallbuys.emplace_back(std::move(object));
  }
  return wallbuys;
}

std::vector<prop_dynamic> World::SpawnDynamicObjects() {

  std::vector<prop_dynamic> dynamicProps;

  for (const Entity &entity : quakeLevel.getEntities()) {
    const auto className = entity.find("classname");
    if (className == entity.end() || className->second != "prop_dynamic") {
      continue;
    }
    const auto model = entity.find("model");
    if (model == entity.end()) {
      TraceLog(LOG_WARNING, "Skipping prop_dynamic without a model");
      continue;
    }

    const std::filesystem::path modelPath = ResolveBspModelPath(model->second);
    if (modelPath.empty()) {
      TraceLog(LOG_WARNING, "Skipping prop_dynamic with invalid model: %s",
               model->second.c_str());
      continue;
    }

    const auto origin = entity.find("origin");
    Vector3 position{};
    if (origin == entity.end() || !ParseVector3(origin->second, position)) {
      TraceLog(LOG_WARNING, "Skipping prop_dynamic with invalid origin");
      continue;
    }

    const std::string modelKey = modelPath.generic_string();
    Model *loadedModel = engine.getAssetManager().LoadModelIfMissing(
        modelKey, modelPath.string());
    if (loadedModel == nullptr) {
      TraceLog(LOG_WARNING, "Could not load prop_dynamic model: %s",
               modelPath.string().c_str());
      continue;
    }

    float entityScale = 1.0f;
    if (const auto scale = entity.find("scale"); scale != entity.end()) {
      try {
        size_t parsedLength = 0;
        entityScale = std::stof(scale->second, &parsedLength);
        if (parsedLength != scale->second.size() ||
            !std::isfinite(entityScale) || entityScale <= 0.0f) {
          throw std::invalid_argument("invalid scale");
        }
      } catch (const std::exception &) {
        TraceLog(LOG_WARNING, "Skipping prop_dynamic with invalid scale: %s",
                 scale->second.c_str());
        continue;
      }
    }

    const BoundingBox bounds = GetModelBoundingBox(*loadedModel);
    const b3Vec3 fullSize{(bounds.max.x - bounds.min.x) * entityScale,
                          (bounds.max.y - bounds.min.y) * entityScale,
                          (bounds.max.z - bounds.min.z) * entityScale};
    if (fullSize.x <= 0.0f || fullSize.y <= 0.0f || fullSize.z <= 0.0f) {
      TraceLog(LOG_WARNING, "Skipping prop_dynamic with empty model bounds: %s",
               modelPath.string().c_str());
      continue;
    }

    const b3Vec3 physicsPosition{position.x, position.y, position.z};
    BoxDef object =
        createDynamicObject(physicsPosition, fullSize, 0.63f, 0.63f, worldId);
    object.type = ModelType::MODEL;
    object.modelPath = modelKey;

    if (const auto angle = entity.find("angle"); angle != entity.end()) {
      try {
        const float radians = std::stof(angle->second) * PI / 180.0f;
        const b3Quat rotation =
            b3MakeQuatFromAxisAngle({0.0f, 1.0f, 0.0f}, radians);
        b3Body_SetTransform(object.bodyId, b3ToPos(physicsPosition), rotation);
      } catch (const std::exception &) {
        TraceLog(LOG_WARNING, "Ignoring invalid prop_dynamic angle: %s",
                 angle->second.c_str());
      }
    }

    dynamicProps.emplace_back(std::move(object), entityScale);
  }
  return dynamicProps;
}

std::vector<info_trigger> World::SpawnTriggers() {

  std::vector<info_trigger> triggers;
  for (const Entity &entity : quakeLevel.getEntities()) {
    const auto className = entity.find("classname");
    if (className == entity.end() || (className->second != "info_trigger")) {
      continue;
    }
    const auto model = entity.find("model");
    if (model == entity.end() || model->second.size() < 2 ||
        model->second.front() != '*') {
      TraceLog(LOG_WARNING, "Skipping info_trigger with invalid model key");
      continue;
    }

    BspModel bspModel{};
    try {
      size_t parsedLength = 0;
      const int modelIndex = std::stoi(model->second.substr(1), &parsedLength);
      if (parsedLength != model->second.size() - 1 || modelIndex <= 0) {
        TraceLog(LOG_WARNING,
                 "Skipping info_trigger with invalid model key: %s",
                 model->second.c_str());
        continue;
      }
      bspModel = quakeLevel.GetModel(static_cast<size_t>(modelIndex));
    } catch (const std::exception &) {
      TraceLog(LOG_WARNING, "Skipping info_trigger with invalid model key: %s",
               model->second.c_str());
      continue;
    }

    const Vector3 center = centerFromQuake(bspModel);
    const Vector3 halfExtents = halfFromQuake(bspModel);
    info_trigger trigger(center, halfExtents, worldId);
    if (auto it = entity.find("target"); it != entity.end()) {
      trigger.target = std::stoi(it->second);
    }
    if (auto it = entity.find("triggered"); it != entity.end()) {
      trigger.triggered = std::stoi(it->second);
    }
    if (auto it = entity.find("enabled"); it != entity.end()) {
      trigger.enabled = std::stoi(it->second);
    }

    triggers.push_back(trigger);
  }
  return triggers;
}
