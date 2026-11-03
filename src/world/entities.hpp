#pragma once

#include "raylib.h"

enum EntityType { ZOMBIE, SKELETON };

struct EntitySpawner {
  Vector3 position;
  EntityType type;
};

struct info_trigger {
  int target = -1;
  int triggered = 0;
  int enabled = 0;
};

// We can store every door in the worlds open state using bitpacking

struct info_reciever {
  int recieverID = -1;
  int triggered = 0;
};
