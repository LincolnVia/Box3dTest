#include "entityRegistry.hpp"

#include "../Engine/engine.hpp"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <utility>

namespace {

bool ParseIntValue(std::string_view text, int &value) {
  const std::string copy(text);
  char *end = nullptr;
  errno = 0;
  const long parsed = std::strtol(copy.c_str(), &end, 10);
  if (end == copy.c_str() || *end != '\0' || errno == ERANGE ||
      parsed < std::numeric_limits<int>::min() ||
      parsed > std::numeric_limits<int>::max()) {
    return false;
  }
  value = static_cast<int>(parsed);
  return true;
}

bool ParseFloatValue(std::string_view text, float &value) {
  const std::string copy(text);
  char *end = nullptr;
  errno = 0;
  const float parsed = std::strtof(copy.c_str(), &end);
  if (end == copy.c_str() || *end != '\0' || errno == ERANGE ||
      !std::isfinite(parsed)) {
    return false;
  }
  value = parsed;
  return true;
}

void LogInvalid(std::string_view className, std::string_view key,
                const std::string &value) {
  TraceLog(LOG_WARNING, "Ignoring invalid %.*s on %.*s: '%s'",
           static_cast<int>(key.size()), key.data(),
           static_cast<int>(className.size()), className.data(),
           value.c_str());
}

} // namespace

const std::string *EntityView::Find(std::string_view key) const {
  const auto entry = entity.find(std::string(key));
  return entry == entity.end() ? nullptr : &entry->second;
}

std::string EntityView::String(std::string_view key, std::string fallback) const {
  const std::string *value = Find(key);
  return value == nullptr ? std::move(fallback) : *value;
}

int EntityView::Int(std::string_view key, int fallback) const {
  int value = fallback;
  return !Has(key) ? fallback : (TryInt(key, value) ? value : fallback);
}

float EntityView::Float(std::string_view key, float fallback) const {
  float value = fallback;
  return !Has(key) ? fallback : (TryFloat(key, value) ? value : fallback);
}

bool EntityView::Bool(std::string_view key, bool fallback) const {
  return Int(key, fallback ? 1 : 0) != 0;
}

Vector3 EntityView::Vector(std::string_view key, Vector3 fallback) const {
  Vector3 value = fallback;
  return !Has(key) ? fallback : (TryVector(key, value) ? value : fallback);
}

bool EntityView::TryInt(std::string_view key, int &value) const {
  const std::string *text = Find(key);
  if (text == nullptr) {
    return false;
  }
  if (!ParseIntValue(*text, value)) {
    LogInvalid(ClassName(), key, *text);
    return false;
  }
  return true;
}

bool EntityView::TryFloat(std::string_view key, float &value) const {
  const std::string *text = Find(key);
  if (text == nullptr) {
    return false;
  }
  if (!ParseFloatValue(*text, value)) {
    LogInvalid(ClassName(), key, *text);
    return false;
  }
  return true;
}

bool EntityView::TryVector(std::string_view key, Vector3 &value) const {
  const std::string *text = Find(key);
  if (text == nullptr) {
    return false;
  }
  if (!ParseVector3(*text, value)) {
    LogInvalid(ClassName(), key, *text);
    return false;
  }
  return true;
}

std::optional<EntityTransform>
ParseEntityTransform(const EntityView &entity, std::string_view className) {
  EntityTransform transform;
  if (!entity.Has("origin")) {
    TraceLog(LOG_WARNING, "Skipping %.*s without an origin",
             static_cast<int>(className.size()), className.data());
    return std::nullopt;
  }
  if (!entity.TryVector("origin", transform.position)) {
    return std::nullopt;
  }

  transform.yaw = entity.Float("angle", 0.0f);
  if (entity.Has("scale") &&
      (!entity.TryFloat("scale", transform.scale) || transform.scale <= 0.0f)) {
    if (transform.scale <= 0.0f) {
      TraceLog(LOG_WARNING, "Skipping %.*s with a non-positive scale",
               static_cast<int>(className.size()), className.data());
    }
    return std::nullopt;
  }
  return transform;
}

std::optional<ModelEntity> ParseModelEntity(Engine &engine,
                                            const EntityView &entity,
                                            std::string_view className) {
  const std::string *model = entity.Find("model");
  if (model == nullptr || model->empty()) {
    TraceLog(LOG_WARNING, "Skipping %.*s without a model",
             static_cast<int>(className.size()), className.data());
    return std::nullopt;
  }

  ModelEntity result;
  result.path = ResolveBspModelPath(*model);
  if (result.path.empty()) {
    TraceLog(LOG_WARNING, "Skipping %.*s with invalid model: %s",
             static_cast<int>(className.size()), className.data(),
             model->c_str());
    return std::nullopt;
  }

  const auto transform = ParseEntityTransform(entity, className);
  if (!transform) {
    return std::nullopt;
  }
  result.transform = *transform;
  result.key = result.path.generic_string();

  Model *loadedModel = engine.getAssetManager().LoadModelIfMissing(
      result.key, result.path.string());
  if (loadedModel == nullptr) {
    TraceLog(LOG_WARNING, "Could not load %.*s model: %s",
             static_cast<int>(className.size()), className.data(),
             result.path.string().c_str());
    return std::nullopt;
  }

  const BoundingBox bounds = GetModelBoundingBox(*loadedModel);
  const float scale = result.transform.scale;
  result.size = {(bounds.max.x - bounds.min.x) * scale,
                 (bounds.max.y - bounds.min.y) * scale,
                 (bounds.max.z - bounds.min.z) * scale};
  if (result.size.x <= 0.0f || result.size.y <= 0.0f ||
      result.size.z <= 0.0f) {
    TraceLog(LOG_WARNING, "Skipping %.*s with empty model bounds: %s",
             static_cast<int>(className.size()), className.data(),
             result.path.string().c_str());
    return std::nullopt;
  }
  return result;
}

void EntityRegistry::Register(std::string className, Handler handler) {
  if (!className.empty() && handler) {
    handlers.insert_or_assign(std::move(className), std::move(handler));
  }
}

void EntityRegistry::Spawn(const Entity &entity) const {
  const std::string className = EntityView(entity).ClassName();
  const auto handler = handlers.find(className);
  if (handler != handlers.end()) {
    handler->second(entity);
  }
}

void EntityRegistry::SpawnAll(const std::vector<Entity> &entities) const {
  for (const Entity &entity : entities) {
    Spawn(entity);
  }
}
