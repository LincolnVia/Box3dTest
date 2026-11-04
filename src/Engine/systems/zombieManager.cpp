#include "zombieManager.hpp"
#include "../../world/quakeBsp.hpp"
void ZombieManager::ReadLevelData(QuakeBspLevel &quakeLevel) {
  for (const Entity &entity : quakeLevel.getEntities()) {
    const auto className = entity.find("classname");
    if (className == entity.end() ||
        (className->second != "info_zombie_spawn")) {
      continue;
    }

    info_zombie_spawn _info_zombie_spawn;

    if (auto it = entity.find("target"); it != entity.end()) {
      _info_zombie_spawn.spawned = std::stoi(it->second);
    }
    if (auto it = entity.find("triggered"); it != entity.end()) {
      _info_zombie_spawn.spawnrate = std::stoi(it->second);
    }
    if (auto it = entity.find(""); it != entity.end()) {
      _info_zombie_spawn.enabled = std::stoi(it->second);
    }
    if (auto it = entity.find("limit"); it != entity.end()) {
      _info_zombie_spawn.limit = std::stoi(it->second);
    }

    triggers.push_back(trigger);
  }
}

void ZombieManager::UpdateSpawners() {}

void ZombieManager::UpdateZombies() {}
