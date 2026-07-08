#ifndef ENGINE_H
#define ENGINE_H

#include "../Debug/debugger.hpp"
#include "../player/player.hpp"
#include "../world/world.hpp"
#include <cstdio>
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
  Engine();
  void Process();
  ~Engine();

private:
  Player player;
  Debugger debugger;
  World world;
};

#endif