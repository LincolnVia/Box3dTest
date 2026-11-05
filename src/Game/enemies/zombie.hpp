#pragma once

#include "box3d/id.h"
#include "box3d/math_functions.h"

namespace game {
class Zombie {
  enum class STATE { IDLE, WALK, CHASE, DEAD };

public:
  Zombie(const Zombie &) = default;
  Zombie(Zombie &&) = default;
  Zombie &operator=(const Zombie &) = default;
  Zombie &operator=(Zombie &&) = default;
  Zombie(b3Vec3 position, b3Vec3 rotation, b3WorldId worldid);
  void Update();
  void CheckPlayerDistance();

private:
  b3Vec3 position;
  b3Vec3 rotation;
  b3BodyId bodyid;
};

} // namespace game
