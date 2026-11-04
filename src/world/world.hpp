#pragma once

#include "../physics/physics.hpp"

#include "../Engine/eventListener.hpp"
#include "cubeBrush.hpp"
#include "entities.hpp"
#include "quakeBsp.hpp"
#include <box3d/box3d.h>
#include <raylib.h>
#include <vector>
class Engine;

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
  std::vector<prop_dynamic> SpawnDynamicObjects();
  std::vector<info_trigger> SpawnTriggers();

  b3WorldId GetWorldId() const { return worldId; }
  std::vector<BoxDef> &GetSceneObjects() { return sceneObjects; }
  bool LoadBspLevel(const std::filesystem::path &path);
  bool HasBspLevel() const { return quakeLevel.IsLoaded(); }
  const std::string &GetBspStatus() const { return bspStatus; }
  Vector3 GetBspPlayerSpawn() const { return quakeLevel.GetPlayerSpawn(); }
  bool HasBspPlayerSpawn() const { return quakeLevel.HasPlayerSpawn(); }

private:
  Engine &engine; // why is this here it should not by im not touching it
  EventListener eventListener;
  std::vector<cubeBrush> meshs;

  Model skyBox;

  b3WorldId worldId{};
  std::vector<BoxDef> sceneObjects;
  QuakeBspLevel quakeLevel;
  std::vector<info_trigger> triggers;
  std::vector<prop_dynamic> dynamicProps;
  std::string bspStatus = "Drop a Quake BSP29 file to load a level";
  bool isShutdown = false;

  float timeStep = 1.0f / 60.0f;
  int subStepCount = 8;
};
