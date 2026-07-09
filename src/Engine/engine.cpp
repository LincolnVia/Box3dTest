#include "engine.hpp"
#include <iostream>
#include <raylib.h>
#define RLIGHTS_IMPLEMENTATION
#include "../rlights.h"

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

  CloseWindow();
}

bool Engine::ShouldClose() const { return WindowShouldClose(); }

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

  // SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
  BeginDrawing();
  ClearBackground(BLUE);
  BeginMode3D(player.getCamera());

  BeginShaderMode(shaderManager.getShader("lighting"));
  world.Draw();
  player.Render();

  EndShaderMode();
  EndMode3D();

  BeginShaderMode(shaderManager.getShader("lighting"));
  player.RenderGun();
  EndShaderMode();

  DrawFPS(10, 10);
  // DrawText("Press F1 to toggle debug mode", 10, 30, 20, WHITE);
  DrawText(TextFormat("Linear Velocity: %.2f, %.2f, %.2f",
                      player.getVelocity().x, player.getVelocity().y,
                      player.getVelocity().z),
           10, 50, 20, WHITE);
  DrawText(TextFormat("Held Object: %s", player.getHeldObject() ? "Yes" : "No"),
           10, 70, 20, WHITE);
  DrawText(
      TextFormat("Held Object Cooldown: %.2f", *player.getHeldObjectCooldown()),
      10, 90, 20, WHITE);

  if (drawOverlay) {
    drawOverlay();
  }

  EndDrawing();
}
