#pragma once

#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "box3d/types.h"
#include "raylib.h"
#include <cstddef>
#include <engine/global.hpp>
#include <engine/physics/physics.hpp>
#include <engine/world/entity.hpp>
#include <game/global.hpp>
#include <utility>
#include <vector>

namespace game {
using engine::DestroyRuntimeBody;
using engine::RuntimeEntity;

enum EntityType { ZOMBIE, SKELETON };

struct EntitySpawner {
  Vector3 position;
  EntityType type;
};

struct wallbuy : RuntimeEntity {
  EnitityData boxdef;
  float modelScale = 1.0f;
  int cost = 0;
  int purchased = 0;
  int enabled = 1;
  wallbuy(EnitityData object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
  ~wallbuy() override { DestroyRuntimeBody(boxdef); }
};
struct prop_dynamic : RuntimeEntity {
  EnitityData boxdef;
  float modelScale = 1.0f;
  int enabled = 1;
  prop_dynamic(EnitityData object, float scale)
      : boxdef(std::move(object)), modelScale(scale) {}
  ~prop_dynamic() override { DestroyRuntimeBody(boxdef); }
};

struct info_zombie_spawn {
  info_zombie_spawn(b3Vec3 _pos) : position(_pos) {};
  b3Vec3 position;
  int spawnrate = 1; // Spawn Zombie Rate
  int limit = -1;    // Max Spawn Ammount | Special Case
  int enabled = 1;   // Should Update
  int spawned = 0;   // Spawn Count Tracker
};

struct info_reciever {
  int recieverID = -1;
  int triggered = 0;
};

} // namespace game
