#include "engine.hpp"
#include <raylib.h>
#define RLIGHTS_IMPLEMENTATION
#include "../rlights.h"

Engine::Engine()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow((int)winSize.x, (int)winSize.y, winTitle);
    SetTargetFPS(60);

    
    debugger.Init();
    shader = LoadShader(
        TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION)
    );

    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, &ambient, SHADER_UNIFORM_VEC4);

    CreateLight(LIGHT_DIRECTIONAL, {0, 0, 0}, {-1, 0, -1}, WHITE, shader);

    world.Init();
    player.Init(world);


}
void Engine::Process()
{
    while (!WindowShouldClose()) {
        world.Update();
        player.Update();
        


        //SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
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

        //debugger.Render();
        EndDrawing();
     

    }
}


Engine::~Engine()
{
    std::cout << "Engine Shutting down" << std::endl;
    
    UnloadShader(shader);
    CloseWindow();
}
