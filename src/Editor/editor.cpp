#include "editor.hpp"

Editor::Editor() : engine("Box3D Editor", false) {
  debugger.Init();
  debugger.world = &engine.getWorld();
}

void Editor::Process() {
  while (!engine.ShouldClose()) {
    engine.ProcessFrame([this]() { debugger.Render(); });
  }
}
