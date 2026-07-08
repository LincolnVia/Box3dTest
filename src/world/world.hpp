#pragma once

#include "../physics/physics.hpp"
#include "../physics/physicsButton.hpp"

#include "cubeBrush.hpp"
#include <box3d/box3d.h>
#include <raylib.h>
#include <vector>
class Engine;

class World {
public:
  explicit World(Engine &engine);
  ~World();

  void Init();
  void Update();
  void Draw();

  b3WorldId GetWorldId() const { return worldId; }
  std::vector<BoxDef> &GetSceneObjects() { return sceneObjects; }
  std::vector<PhysicsButton> &GetSceneButtons() { return sceneButtonsObjects; }

private:
  void ApplyCompanionCubeReleasedTuning();

  Engine &engine;
  std::vector<cubeBrush> meshs;
  Texture2D grassTex{};
  Texture2D wallTex{};
  Model m_cube{};
  Model m_button{};
  Model m_world{};
  Model skyBox;

  b3WorldId worldId{};
  std::vector<BoxDef> sceneObjects;
  std::vector<PhysicsButton> sceneButtonsObjects;

  float timeStep = 1.0f / 60.0f;
  int subStepCount = 8;
};
