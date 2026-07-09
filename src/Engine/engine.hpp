#ifndef ENGINE_H
#define ENGINE_H

#include "../player/player.hpp"
#include "../world/world.hpp"
#include "assetManager.hpp"
#include "shaderManager.hpp"

#include <cstdio>
#include <functional>
#include <iostream>
#include <ostream>
#include <raylib.h>
#include <raymath.h>

class Engine {
public:
  Vector2 winSize = {1280, 720};
  const char *winTitle = "Box3D";

  explicit Engine(const char *title = "Box3D", bool enablePlayerInput = true);
  AssetManager &getAssetManager() { return assetManager; }
  ShaderManager &getShaderManager() { return shaderManager; }
  World &getWorld() { return world; }
  bool ShouldClose() const;
  void Process();
  void ProcessFrame(const std::function<void()> &drawOverlay = nullptr);
  ~Engine();

private:
  bool enablePlayerInput = true;
  AssetManager assetManager;
  ShaderManager shaderManager;
  Player player;
  World world;
};

#endif
