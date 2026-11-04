#pragma once

struct HealthComponent {
  int health;
  int getHealth() const { return health; }
  void setHealth(int val) { health = val; }
};
