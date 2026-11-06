#pragma once

#include <engine/physics/physics.hpp>

#include "entities.hpp"
#include "game/systems/zombieManager.hpp"
#include <box3d/box3d.h>
#include <engine/eventListener.hpp>
#include <engine/world/cubeBrush.hpp>
#include <engine/world/entityRegistry.hpp>
#include <engine/world/quakeBsp.hpp>
#include <memory>
#include <raylib.h>
#include <vector>

namespace engine {
class Engine;
}

namespace game {
using engine::Engine;
using engine::EventListener;

class World {
public:
  explicit World(Engine &engine);
  ~World();

  void Init();
  void Shutdown();
  void Update();
  void Draw();
  void DrawGameObjects();
  void SpawnEntities();
  void Interact(b3BodyId bodyId);
  b3WorldId GetWorldId() const { return worldId; }
  std::vector<EnitityData> &GetSceneObjects() { return sceneObjects; }
  bool LoadBspLevel(const std::filesystem::path &path);
  bool HasBspLevel() const { return quakeLevel.IsLoaded(); }
  const std::string &GetBspStatus() const { return bspStatus; }
  SpawnData GetBspPlayerSpawn() const { return quakeLevel.GetPlayerSpawn(); }
  bool HasBspPlayerSpawn() const { return quakeLevel.HasPlayerSpawn(); }
  std::vector<std::unique_ptr<RuntimeEntity>> &getRuntimeEntities() {
    return runtimeEntities;
  }
  void setRuntimeEntities(int index, std::unique_ptr<RuntimeEntity> val) {
    runtimeEntities[index].swap(val);
  }

private:
  void RegisterEntitySpawners();
  void ClearSpawnedEntities();
  void SpawnTrigger(const Entity &entity);
  void SpawnDynamicProp(const Entity &entity);
  void SpawnWallbuy(const Entity &entity);

  Engine &engine; // why is this here it should not by im not touching it
  EventListener eventListener;
  ZombieManager zombieManager;
  EntityRegistry entitySpawners;
  std::vector<cubeBrush> meshs;

  Model skyBox;

  b3WorldId worldId{};
  std::vector<EnitityData> sceneObjects;
  QuakeBspLevel quakeLevel;
  std::vector<std::unique_ptr<RuntimeEntity>> runtimeEntities;

  std::string bspStatus = "Drop a Quake BSP29 file to load a level";
  bool isShutdown = false;

  float timeStep = 1.0f / 60.0f;
  int subStepCount = 8;
};

} // namespace game
