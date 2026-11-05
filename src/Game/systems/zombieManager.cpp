#include "zombieManager.hpp"

#include "../../world/entityRegistry.hpp"
#include "../../world/quakeBsp.hpp"

namespace game {
void ZombieManager::ReadLevelData(QuakeBspLevel &quakeLevel) {
  for (const Entity &entity : quakeLevel.getEntities()) {
    const EntityView properties(entity);
    if (properties.ClassName() != "info_zombie_spawn") {
      continue;
    }

    const Vector3 position = properties.Vector("origin", Vector3{});
    info_zombie_spawn _info_zombie_spawn{{position.x, position.y, position.z}};
    _info_zombie_spawn.spawnrate = properties.Int("spawnrate", 1);
    _info_zombie_spawn.enabled = properties.Int("enabled", 1);
    _info_zombie_spawn.limit = properties.Int("limit", -1);

    zombieSpawners.push_back(_info_zombie_spawn);
  }
}

void ZombieManager::UpdateSpawners() {}

void ZombieManager::UpdateZombies() {}

} // namespace game
