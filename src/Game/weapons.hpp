#pragma once

#include "box3d/math_functions.h"
#include <string>
namespace game {
enum WEAPON_NAME {
  M1911,
  AK47,
  REMMINGTON,

};
struct weapon_stats {
  std::string name;
  int clip_size = 9;
  float fire_speed = 2;
  b3Vec2 recoil_pattern[];
};

} // namespace game
