#pragma once

namespace game {
struct HealthComponent {
  int health;
  int getHealth() const { return health; }
  void setHealth(int val) { health = val; }
};

} // namespace game
