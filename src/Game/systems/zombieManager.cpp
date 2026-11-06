#include "zombieManager.hpp"
#include "box3d/math_functions.h"
#include "engine/graphics/graphics.hpp"
#include "game/world/entities.hpp"
#include "raylib.h"

#include <engine/world/entityRegistry.hpp>
#include <engine/world/quakeBsp.hpp>

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

void ZombieManager::UpdateSpawners() {
  for (info_zombie_spawn &_info_zombie_spawn : zombieSpawners) {
    DrawCube(B3Vec3ToVector3(_info_zombie_spawn.position), 5, 5, 5, RED);
  }
}

void ZombieManager::UpdateZombies() {}

} // namespace game
