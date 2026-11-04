#pragma once

#include "../../world/entities.hpp"
#include "../../world/quakeBsp.hpp"
#include "../Enemies/zombie.hpp"
#include <vector>
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
