#pragma once

#include <game/world/entities.hpp>
#include <engine/world/quakeBsp.hpp>
#include <game/enemies/zombie.hpp>
#include <vector>

namespace game {
class ZombieManager {
public:
  void ReadLevelData(QuakeBspLevel &quakeLevel);
  void UpdateSpawners();
  void UpdateZombies();

private:
  std::vector<info_zombie_spawn> zombieSpawners;
  std::vector<Zombie> ActiveZombies;
  std::vector<Zombie> inActiveZombies;
};

} // namespace game
