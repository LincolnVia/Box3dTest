#pragma once

#include <Debug/debugger.hpp>
#include <engine/engine.hpp>

class Editor {
public:
  Editor();
  void Process();

private:
  engine::Engine engine;
  Debugger debugger;
};
