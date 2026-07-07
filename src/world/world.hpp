#pragma once

#include "cubeBrush.hpp"
#include "../physics.hpp"
#include <box3d/box3d.h>
#include <raylib.h>
#include <vector>

class World
{
public:
    World();
    ~World();

    void Init();
    void Update();
    void Draw();
    const Model& GetCompanionCubeModel() const { return m_cube; };
    const Model& GetWorldMesh() const { return m_world; };
    const BoxDef& GetCompanionCube() const { return companionCube; }
    
    b3WorldId GetWorldId() const { return worldId; }
private:
    std::vector<cubeBrush> meshs;
    Texture2D grassTex{};
    Texture2D wallTex{};
    Model m_cube{};
    Model m_world{};

    b3WorldId worldId{};
    std::vector<BoxDef> staticObjects;
    
    BoxDef companionCube{};

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;
};
