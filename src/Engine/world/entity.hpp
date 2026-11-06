#pragma once

#include <box3d/id.h>
#include <engine/physics/physics.hpp>

namespace engine {

class RuntimeEntity {
public:
  RuntimeEntity() = default;
  RuntimeEntity(const b3BodyId &playerId);
  RuntimeEntity(const RuntimeEntity &) = delete;
  RuntimeEntity &operator=(const RuntimeEntity &) = delete;
  virtual ~RuntimeEntity() = default;
  virtual void Update() {}
  virtual void Update(const b3BodyId &playerId) {}
  virtual void Interact() {}
};

inline void DestroyRuntimeBody(game::EnitityData &object) {
  if (B3_IS_NON_NULL(object.bodyId)) {
    b3DestroyBody(object.bodyId);
    object.bodyId = b3_nullBodyId;
  }
}
class PhysicsEntity : public RuntimeEntity {
public:
  PhysicsEntity(b3BodyId id) : bodyId(id) {}

private:
  b3BodyId bodyId;
};

} // namespace engine
