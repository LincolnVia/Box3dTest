#pragma once

#include "../physics/physics.hpp"
#include "cubeBrush.hpp"
#include <box3d/box3d.h>
#include <raylib.h>
#include <vector>

class World {
public:
  World();
  ~World();

  void Init();
  void Update();
  void Draw();

  b3WorldId GetWorldId() const { return worldId; }
  std::vector<BoxDef> &GetSceneObjects() { return sceneObjects; }

private:
  std::vector<cubeBrush> meshs;
  Texture2D grassTex{};
  Texture2D wallTex{};
  Model m_cube{};
  Model m_world{};

  b3WorldId worldId{};
  std::vector<BoxDef> sceneObjects;

  float timeStep = 1.0f / 60.0f;
  int subStepCount = 4;
};
