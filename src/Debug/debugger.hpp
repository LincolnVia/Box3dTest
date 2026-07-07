#pragma once


#include <rlimgui.h>
#include "../world/world.hpp"

class Debugger
{
    public:
    void Init();
    ~Debugger();
    
    World* world = nullptr;
    
    void Update();
    void Render();


};