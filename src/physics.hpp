#pragma once

#include <box3d/box3d.h>
#include <box3d/math_functions.h>
#include <cstdint>
#include <string>

namespace CollisionCategory
{
    constexpr uint64_t Static = 1ull << 0;
    constexpr uint64_t Dynamic = 1ull << 1;
    constexpr uint64_t Player = 1ull << 2;
    constexpr uint64_t World = Static | Dynamic;
    constexpr uint64_t All = Static | Dynamic | Player;
}

struct BoxDef
{
    std::string name;
    b3BodyId bodyId{};
    b3ShapeId shapeId{};
    b3BoxHull hull{};
    b3Vec3 halfExtents{};
};
