#pragma once

#include "../physics/physics.hpp"
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/id.h"
#include "box3d/types.h"
#include "raylib.h"
#include <xlocale/_stdio.h>
enum EntityType { ZOMBIE, SKELETON };

struct EntitySpawner {
  Vector3 position;
  EntityType type;
};

struct info_trigger {
  b3ShapeDef shapeDef;
  b3BodyId bodyid;
  b3BodyDef bodydef;
  b3ShapeId shapeid;
  BspModel *bspModel;
  int target = -1;
  int triggered = 0;
  int enabled = 1;
  info_trigger(BspModel *_bspModel, b3WorldId &worldId) : bspModel(_bspModel) {
    bodydef.position = {bspModel->origin[0], bspModel->origin[1],
                        bspModel->origin[2]};
    bodydef.type = b3BodyType::b3_staticBody;

    shapeDef = b3DefaultShapeDef();
    shapeDef.filter.categoryBits = CollisionCategory::Trigger;
    shapeDef.isSensor = true;
    shapeDef.enableSensorEvents = true;
    b3BoxHull hull = b3MakeBoxHull(bspModel->maxs[0] / 2, bspModel->maxs[1] / 2,
                                   bspModel->maxs[2] / 2);
    bodyid = b3CreateBody(worldId, &bodydef);
    b3ShapeId info_trigger_shape_id =
        b3CreateHullShape(bodyid, &shapeDef, &hull.base);
  }
  bool on_trigger_collided() {
    if (enabled)
      triggered = 1;
  }
  bool trigger_player_enter() {}
};

struct info_reciever {
  int recieverID = -1;
  int triggered = 0;
};
