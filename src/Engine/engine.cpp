#include "engine.hpp"
#include <raylib.h>
#define RLIGHTS_IMPLEMENTATION
#include "../rlights.h"

Engine::Engine() : world(*this) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow((int)winSize.x, (int)winSize.y, winTitle);
  SetTargetFPS(60);

  debugger.Init();
  shader = LoadShader(
      TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
      TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
  shdrCubemap = LoadShader(
      TextFormat("resources/shaders/glsl%i/cubemap.vs", GLSL_VERSION),
      TextFormat("resources/shaders/glsl%i/cubemap.fs", GLSL_VERSION));

  shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
  shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

  int ambientLoc = GetShaderLocation(shader, "ambient");
  SetShaderValue(shader, ambientLoc, &ambient, SHADER_UNIFORM_VEC4);

  CreateLight(LIGHT_DIRECTIONAL, {0, 0, 0}, {-1, 0, -1}, WHITE, shader);
  assetManager.Init();
  world.Init();
  player.Init(world);
}
Engine::~Engine() {
  std::cout << "Engine Shutting down" << std::endl;

  UnloadShader(shader);
  UnloadShader(shdrCubemap);

  CloseWindow();
}

void Engine::Process() {
  while (!WindowShouldClose()) {
    world.Update();
    player.Update();

    // SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
    BeginDrawing();
    ClearBackground(BLUE);
    BeginMode3D(player.getCamera());

    BeginShaderMode(shader);
    world.Draw();
    player.Render();

    EndShaderMode();
    EndMode3D();

    BeginShaderMode(shader);
    player.RenderGun();
    EndShaderMode();

    DrawFPS(10, 10);
    // DrawText("Press F1 to toggle debug mode", 10, 30, 20, WHITE);
    DrawText(TextFormat("Linear Velocity: %.2f, %.2f, %.2f",
                        player.getVelocity().x, player.getVelocity().y,
                        player.getVelocity().z),
             10, 50, 20, WHITE);
    DrawText(
        TextFormat("Held Object: %s", player.getHeldObject() ? "Yes" : "No"),
        10, 70, 20, WHITE);
    DrawText(TextFormat("Held Object Cooldown: %.2f",
                        *player.getHeldObjectCooldown()),
             10, 90, 20, WHITE);
    // debugger.Render();
    EndDrawing();
  }
}
