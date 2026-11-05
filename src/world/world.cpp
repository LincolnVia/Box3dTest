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
  RegisterEntitySpawners();
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
  ClearSpawnedEntities();
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
        engine.LoadBspLevel(path);
      } else if (extension != ".bsp") {
        TraceLog(LOG_WARNING, "Dropped file is not a .bsp: %s",
                 path.string().c_str());
      }
    }

    UnloadDroppedFiles(droppedFiles);
  }
  b3World_Step(worldId, timeStep, subStepCount);
  for (const auto &entity : runtimeEntities) {
    entity->Update();
  }
}

void World::Interact(b3BodyId bodyId) {
  for (const auto &entity : runtimeEntities) {
    const BoxDef *object = nullptr;
    if (const auto *trigger = dynamic_cast<const info_trigger *>(entity.get())) {
      object = &trigger->boxdef;
    } else if (const auto *prop =
                   dynamic_cast<const prop_dynamic *>(entity.get())) {
      object = &prop->boxdef;
    } else if (const auto *buy = dynamic_cast<const wallbuy *>(entity.get())) {
      object = &buy->boxdef;
    }
    if (object != nullptr && B3_ID_EQUALS(object->bodyId, bodyId)) {
      entity->Interact();
      return;
    }
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
  for (const auto &entity : runtimeEntities) {
    const BoxDef *object = nullptr;
    float scale = 1.0f;
    if (const auto *prop = dynamic_cast<const prop_dynamic *>(entity.get())) {
      object = &prop->boxdef;
      scale = prop->modelScale;
    } else if (const auto *buy = dynamic_cast<const wallbuy *>(entity.get())) {
      object = &buy->boxdef;
      scale = buy->modelScale;
    }
    if (object != nullptr) {
      const b3Vec3 modelScale{scale, scale, scale};
      b3_DrawModel(engine.getAssetManager().getModel(object->modelPath),
                   B3WorldTransformToMatrix(
                       b3Body_GetTransform(object->bodyId), modelScale));
    }
  }
}
void World::Draw() {
  DrawSkybox(skyBox);
  quakeLevel.Draw(engine.getShaderManager().getShader("lighting"));
  DrawGameObjects();
}

void World::SpawnEntities() {
  ClearSpawnedEntities();
  entitySpawners.SpawnAll(quakeLevel.getEntities());
}

void World::RegisterEntitySpawners() {
  entitySpawners.Register("info_trigger", [this](const Entity &entity) {
    SpawnTrigger(entity);
  });
  entitySpawners.Register("prop_dynamic", [this](const Entity &entity) {
    SpawnDynamicProp(entity);
  });

  for (const char *className : {"wallbuy", "weapon_1911", "weapon_ak47",
                                "weapon_remmington"}) {
    entitySpawners.Register(className, [this](const Entity &entity) {
      SpawnWallbuy(entity);
    });
  }
}

void World::ClearSpawnedEntities() {
  runtimeEntities.clear();
}

namespace {
b3Vec3 ToPhysicsPosition(const Vector3 &position) {
  return {position.x, position.y, position.z};
}

void ApplyYaw(const BoxDef &object, const EntityTransform &transform) {
  if (transform.yaw == 0.0f) {
    return;
  }
  const b3Quat rotation = b3MakeQuatFromAxisAngle(
      {0.0f, 1.0f, 0.0f}, transform.yaw * PI / 180.0f);
  b3Body_SetTransform(object.bodyId,
                      b3ToPos(ToPhysicsPosition(transform.position)), rotation);
}

BoxDef CreateModelObject(b3WorldId worldId, const ModelEntity &model) {
  const b3Vec3 position = ToPhysicsPosition(model.transform.position);
  BoxDef object =
      createDynamicObject(position, model.size, 0.63f, 0.63f, worldId);
  object.type = ModelType::MODEL;
  object.modelPath = model.key;
  ApplyYaw(object, model.transform);
  return object;
}
} // namespace

void World::SpawnDynamicProp(const Entity &entity) {
  const auto model =
      ParseModelEntity(engine, EntityView(entity), "prop_dynamic");
  if (!model) {
    return;
  }
  runtimeEntities.push_back(std::make_unique<prop_dynamic>(
      CreateModelObject(worldId, *model), model->transform.scale));
}

void World::SpawnWallbuy(const Entity &entity) {
  const EntityView view(entity);
  const auto model = ParseModelEntity(engine, view, "wallbuy");
  if (!model) {
    return;
  }

  auto buy = std::make_unique<wallbuy>(CreateModelObject(worldId, *model),
                                       model->transform.scale);
  buy->cost = view.Int("cost", 0);
  buy->purchased = view.Int("purchased", 0);
  runtimeEntities.push_back(std::move(buy));
}

void World::SpawnTrigger(const Entity &entity) {
  const EntityView view(entity);
  const std::string *model = view.Find("model");
  if (model == nullptr || model->size() < 2 || model->front() != '*') {
    TraceLog(LOG_WARNING, "Skipping info_trigger with invalid model key");
    return;
  }

  BspModel bspModel{};
  try {
    size_t parsedLength = 0;
    const int modelIndex = std::stoi(model->substr(1), &parsedLength);
    if (parsedLength != model->size() - 1 || modelIndex <= 0) {
      TraceLog(LOG_WARNING, "Skipping info_trigger with invalid model key: %s",
               model->c_str());
      return;
    }
    bspModel = quakeLevel.GetModel(static_cast<size_t>(modelIndex));
  } catch (const std::exception &) {
    TraceLog(LOG_WARNING, "Skipping info_trigger with invalid model key: %s",
             model->c_str());
    return;
  }

  auto trigger = std::make_unique<info_trigger>(
      centerFromQuake(bspModel), halfFromQuake(bspModel), worldId);
  trigger->target = view.Int("target", -1);
  trigger->triggered = view.Int("triggered", 0);
  trigger->enabled = view.Int("enabled", 1);
  runtimeEntities.push_back(std::move(trigger));
}
