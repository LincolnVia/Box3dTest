#pragma once

#include <box3d/box3d.h>
#include <raylib.h>

struct cubeBrush
{
    b3WorldTransform transform{};
    b3Vec3 scale{};
    Color color{};
};
