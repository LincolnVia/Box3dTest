#pragma once

#include <box3d/box3d.h>
#include <raylib.h>

#include <filesystem>
#include <string>
#include <vector>

struct b3MeshData;

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

private:
  std::vector<Model> models;
  std::vector<Texture2D> textures;
  b3BodyId collisionBodyId = b3_nullBodyId;
  b3MeshData *collisionMesh = nullptr;
  std::string sourcePath;
  Vector3 playerSpawn{};
  bool hasPlayerSpawn = false;
  int triangleCount = 0;
};
