#pragma once

#include "../world/world.hpp"
#include <rlimgui.h>


class Debugger {
public:
  void Init();
  ~Debugger();

  World *world = nullptr;

  void Update();
  void Render();
};