#pragma once

#include "entities.hpp"
#include "world/quakeBsp.hpp"
#include <vector>
bool FindPlayerSpawn(const std::vector<Entity> &entities, Vector3 &spawn) {
  for (const Entity &entity : entities) {
    const auto className = entity.find("classname");
    if (className == entity.end() ||
        (className->second != "info_player_start" &&
         className->second != "info_player_deathmatch")) {
      continue;
    }
    const auto origin = entity.find("origin");
    if (origin != entity.end() && ParseVector3(origin->second, spawn)) {
      spawn.y += 1.0f;
      return true;
    }
  }
  return false;
}
// I need a event listener
