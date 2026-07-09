#ifndef ENGINE_H
#define ENGINE_H

#include "../player/player.hpp"
#include "../world/world.hpp"
#include "assetManager.hpp"

#include <cstdio>
#include <functional>
#include <iostream>
#include <ostream>
#include <raylib.h>
#include <raymath.h>

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

class Engine {
public:
  Vector2 winSize = {1280, 720};
  const char *winTitle = "Box3D";
  Shader shader = {};
  Shader shdrCubemap = {};
  Vector4 ambient = {1.0f, 1.0f, 1.0f, 1.0f};
  explicit Engine(const char *title = "Box3D", bool enablePlayerInput = true);
  AssetManager &getAssetManager() { return assetManager; }
  World &getWorld() { return world; }
  bool ShouldClose() const;
  void Process();
  void ProcessFrame(const std::function<void()> &drawOverlay = nullptr);
  ~Engine();

private:
  bool enablePlayerInput = true;
  AssetManager assetManager;
  Player player;
  World world;
};

#endif
