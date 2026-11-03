#pragma once

#include <box3d/box3d.h>
#include <raylib.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

struct b3MeshData;
using Entity = std::unordered_map<std::string, std::string>;

bool ParseVector3(std::string_view text, Vector3 &result);

// A renderable Quake 1 BSP29 world backed by a static Box3D triangle mesh.
class QuakeBspLevel {
public:
  QuakeBspLevel() = default;
  QuakeBspLevel(const QuakeBspLevel &) = delete;
  QuakeBspLevel &operator=(const QuakeBspLevel &) = delete;

  bool Load(const std::filesystem::path &path, b3WorldId worldId,
            std::string &error);
  void Unload();
  void Draw(Shader shader);

  bool IsLoaded() const { return !models.empty(); }
  const std::string &GetPath() const { return sourcePath; }
  bool HasPlayerSpawn() const { return hasPlayerSpawn; }
  Vector3 GetPlayerSpawn() const { return playerSpawn; }
  int GetTriangleCount() const { return triangleCount; }
  const std::vector<Entity> &getEntities() const { return entities; }
  void setEntities(std::vector<Entity> val) { entities = val; }

private:
  struct StaticModelInstance {
    size_t modelIndex = 0;
    Vector3 position{};
    float yaw = 0.0f;
    float scale = 1.0f;
  };

  std::vector<Model> models;
  std::vector<Texture2D> textures;
  std::vector<Model> staticModels;
  std::vector<Entity> entities;
  std::vector<StaticModelInstance> staticModelInstances;
  b3BodyId collisionBodyId = b3_nullBodyId;
  b3MeshData *collisionMesh = nullptr;
  std::string sourcePath;
  Vector3 playerSpawn{};
  bool hasPlayerSpawn = false;
  int triangleCount = 0;
};
