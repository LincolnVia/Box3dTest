#ifndef ENGINE_H
#define ENGINE_H

#include <game/player/player.hpp>
#include <game/world/world.hpp>
#include "assetManager.hpp"
#include "eventListener.hpp"
#include "shaderManager.hpp"

#include <cstdio>
#include <filesystem>
#include <functional>
#include <iostream>
#include <ostream>
#include <raylib.h>
#include <raymath.h>

namespace engine {

class Engine {
public:
  Vector2 winSize = {1280, 720};
  const char *winTitle = "Box3D";

  explicit Engine(const char *title = "Box3D", bool enablePlayerInput = true);
  AssetManager &getAssetManager() { return assetManager; }
  ShaderManager &getShaderManager() { return shaderManager; }
  game::World &getWorld() { return world; }
  bool ShouldClose() const;
  bool LoadBspLevel(const std::filesystem::path &path);
  void Process();
  void ProcessFrame(const std::function<void()> &drawOverlay = nullptr);
  ~Engine();

private:
  bool enablePlayerInput = true;
  AssetManager assetManager;
  ShaderManager shaderManager;

  game::Player player;
  game::World world;
};


} // namespace engine
#endif
