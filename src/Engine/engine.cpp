#include "engine.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
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

  world.Shutdown();
  CloseWindow();
}

bool Engine::ShouldClose() const { return WindowShouldClose(); }

bool Engine::LoadBspLevel(const std::filesystem::path &path) {
  if (!world.LoadBspLevel(path)) {
    return false;
  }
  if (world.HasBspPlayerSpawn()) {
    player.Teleport(world.GetBspPlayerSpawn());
  }
  return true;
}

void Engine::Process() {
  while (!ShouldClose()) {
    ProcessFrame();
  }
}

void Engine::ProcessFrame(const std::function<void()> &drawOverlay) {
  if (IsFileDropped()) {
    FilePathList droppedFiles = LoadDroppedFiles();
    if (droppedFiles.count > 0) {
      const std::filesystem::path path(droppedFiles.paths[0]);
      std::string extension = path.extension().string();
      std::transform(extension.begin(), extension.end(), extension.begin(),
                     [](unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                     });
      if (extension == ".bsp") {
        LoadBspLevel(path);
      } else if (extension != ".bsp") {
        TraceLog(LOG_WARNING, "Dropped file is not a .bsp: %s",
                 path.string().c_str());
      }
    }
    UnloadDroppedFiles(droppedFiles);
  }

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
  DrawText(world.GetBspStatus().c_str(), 10, 115, 18, WHITE);

  if (drawOverlay) {
    drawOverlay();
  }

  EndDrawing();
}
