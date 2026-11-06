#pragma once

#include "quakeBsp.hpp"
#include <box3d/box3d.h>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine { class Engine; }

namespace game {
using engine::Engine;


class EntityView {
public:
  explicit EntityView(const Entity &entity) : entity(entity) {}

  const std::string *Find(std::string_view key) const;
  bool Has(std::string_view key) const { return Find(key) != nullptr; }
  std::string String(std::string_view key, std::string fallback = {}) const;
  int Int(std::string_view key, int fallback = 0) const;
  float Float(std::string_view key, float fallback = 0.0f) const;
  bool Bool(std::string_view key, bool fallback = false) const;
  Vector3 Vector(std::string_view key, Vector3 fallback = {}) const;
  bool TryInt(std::string_view key, int &value) const;
  bool TryFloat(std::string_view key, float &value) const;
  bool TryVector(std::string_view key, Vector3 &value) const;
  std::string ClassName() const { return String("classname"); }

private:
  const Entity &entity;
};

struct EntityTransform {
  Vector3 position{};
  float yaw = 0.0f;
  float scale = 1.0f;
};

struct ModelEntity {
  EntityTransform transform;
  std::filesystem::path path;
  std::string key;
  b3Vec3 size{};
};

std::optional<EntityTransform>
ParseEntityTransform(const EntityView &entity, std::string_view className);
std::optional<ModelEntity> ParseModelEntity(Engine &engine,
                                           const EntityView &entity,
                                           std::string_view className);

class EntityRegistry {
public:
  using Handler = std::function<void(const Entity &)>;

  void Register(std::string className, Handler handler);
  void Spawn(const Entity &entity) const;
  void SpawnAll(const std::vector<Entity> &entities) const;

private:
  std::unordered_map<std::string, Handler> handlers;
};

} // namespace game
