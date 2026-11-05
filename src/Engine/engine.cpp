#include "engine.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <raylib.h>
#define RLIGHTS_IMPLEMENTATION
#include "../rlights.h"

namespace engine {

Engine::Engine(const char *title, bool enablePlayerInput)
    : winTitle(title), enablePlayerInput(enablePlayerInput), world(*this) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow((int)winSize.x, (int)winSize.y, winTitle);
  SetTargetFPS(60);
  shaderManager.Init();
  assetManager.Init();

  world.Init();
  player.Init(world, enablePlayerInput);
}
Engine::~Engine() {
  std::cout << "Engine Shutting down" << std::endl;

  world.Shutdown();

  CloseWindow();
}

bool Engine::ShouldClose() const { return WindowShouldClose(); }

bool Engine::LoadBspLevel(const std::filesystem::path &path) {
  if (!world.LoadBspLevel(path)) {
    return false;
  }
  if (world.HasBspPlayerSpawn()) {
    player.Teleport(world.GetBspPlayerSpawn().position,
                    world.GetBspPlayerSpawn().rotation);
    TraceLog(LOG_INFO, "Player spawn angle: %f",
             world.GetBspPlayerSpawn().rotation);
  }
  return true;
}

void Engine::Process() {
  while (!ShouldClose()) {
    ProcessFrame();
  }
}

void Engine::ProcessFrame(const std::function<void()> &drawOverlay) {

  world.Update();
  if (enablePlayerInput) {
    player.Update();
  }

  BeginShaderMode(shaderManager.getShader("lighting"));
  player.RenderGunTexture();
  EndShaderMode();

  // SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
  ClearBackground(BLUE);
  BeginMode3D(player.getCamera());

  BeginShaderMode(shaderManager.getShader("lighting"));
  world.Draw();
  player.Render();

  EndShaderMode();
  EndMode3D();

  constexpr int hudFontSize = 8;
  DrawText(TextFormat("%i FPS", GetFPS()), 3, 3, hudFontSize, LIME);
  DrawText(TextFormat("Velocity: %.2f, %.2f, %.2f", player.getVelocity().x,
                      player.getVelocity().y, player.getVelocity().z),
           3, 15, hudFontSize, WHITE);
  DrawText(TextFormat("Held Object: %s", player.getHeldObject() ? "Yes" : "No"),
           3, 27, hudFontSize, WHITE);
  DrawText(TextFormat("Held Cooldown: %.2f", *player.getHeldObjectCooldown()),
           3, 39, hudFontSize, WHITE);
  DrawText(world.GetBspStatus().c_str(), 3, 51, hudFontSize, WHITE);

  // Draw the target texture stretched across the screen with point filtering
  if (drawOverlay) {
    drawOverlay();
  }

  EndDrawing();
}

} // namespace engine
