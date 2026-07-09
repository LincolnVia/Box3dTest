#pragma once

#include "../Debug/debugger.hpp"
#include "../Engine/engine.hpp"

class Editor {
public:
  Editor();
  void Process();

private:
  Engine engine;
  Debugger debugger;
};
