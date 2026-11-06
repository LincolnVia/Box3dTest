#pragma once

#include <game/world/world.hpp>
#include <rlImGui.h>


class Debugger {
public:
  void Init();
  ~Debugger();

  game::World *world = nullptr;

  void Update();
  void Render();
};
