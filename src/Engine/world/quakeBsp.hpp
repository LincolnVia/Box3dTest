#pragma once

#include <engine/physics/physics.hpp>
#include "quakePalette.hpp"
#include <array>
#include <box3d/box3d.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <raylib.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct b3MeshData;

namespace game {
using Entity = std::unordered_map<std::string, std::string>;

constexpr int32_t kQuakeBspVersion = 29;

struct BspModel {
  float mins[3];
  float maxs[3];
  float origin[3];
  int32_t headNodes[4];
  int32_t visibleLeaves;
  int32_t firstFace;
  int32_t faceCount;
};

struct SpawnData {
  Vector3 position;
  float rotation;
};

#pragma pack(push, 1)
struct Lump {
  int32_t offset;
  int32_t size;
};

struct BspHeader {
  int32_t version;
  Lump entities;
  Lump planes;
  Lump mipTextures;
  Lump vertices;
  Lump visibility;
  Lump nodes;
  Lump textureInfo;
  Lump faces;
  Lump lighting;
  Lump clipNodes;
  Lump leaves;
  Lump markSurfaces;
  Lump edges;
  Lump surfEdges;
  Lump models;
};

struct BspVertex {
  float x, y, z;
};

struct BspEdge {
  uint16_t vertices[2];
};

struct BspTextureInfo {
  float uAxis[3];
  float uOffset;
  float vAxis[3];
  float vOffset;
  uint32_t mipTexture;
  uint32_t flags;
};

struct BspFace {
  uint16_t plane;
  uint16_t side;
  int32_t firstSurfEdge;
  uint16_t edgeCount;
  uint16_t textureInfo;
  uint8_t lightStyles[4];
  uint32_t lightmapOffset;
};

struct BspMipTexture {
  char name[16];
  uint32_t width;
  uint32_t height;
  uint32_t offsets[4];
};
#pragma pack(pop)

static_assert(sizeof(BspHeader) == 124);
static_assert(sizeof(BspFace) == 20);

bool ParseVector3(std::string_view text, Vector3 &result);
Vector3 FromQuake(float x, float y, float z);
std::filesystem::path ResolveBspModelPath(std::string modelName);

class BspData {
public:
  explicit BspData(const std::filesystem::path &path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) {
      throw std::runtime_error("could not open BSP file");
    }

    const std::streamoff length = stream.tellg();
    if (length < static_cast<std::streamoff>(sizeof(BspHeader))) {
      throw std::runtime_error("file is too small to contain a BSP header");
    }
    if (length >
        static_cast<std::streamoff>(std::numeric_limits<int32_t>::max())) {
      throw std::runtime_error("BSP file is too large");
    }

    bytes.resize(static_cast<size_t>(length));
    stream.seekg(0);
    stream.read(reinterpret_cast<char *>(bytes.data()), length);
    if (!stream) {
      throw std::runtime_error("failed while reading BSP file");
    }

    std::memcpy(&header, bytes.data(), sizeof(header));
    if (header.version != kQuakeBspVersion) {
      throw std::runtime_error(
          "unsupported BSP version (expected Quake BSP29)");
    }

    const std::array<Lump, 15> lumps = {
        header.entities,    header.planes,     header.mipTextures,
        header.vertices,    header.visibility, header.nodes,
        header.textureInfo, header.faces,      header.lighting,
        header.clipNodes,   header.leaves,     header.markSurfaces,
        header.edges,       header.surfEdges,  header.models,
    };
    for (const Lump lump : lumps) {
      ValidateRange(lump.offset, lump.size, "invalid BSP lump range");
    }
  }

  template <typename T> T At(Lump lump, size_t index) const {
    if (index >= Count<T>(lump)) {
      throw std::runtime_error("BSP lump index is out of range");
    }
    T value{};
    std::memcpy(&value, bytes.data() + lump.offset + index * sizeof(T),
                sizeof(T));
    return value;
  }

  template <typename T> size_t Count(Lump lump) const {
    if (lump.size % static_cast<int32_t>(sizeof(T)) != 0) {
      throw std::runtime_error("BSP lump has an invalid record size");
    }
    return static_cast<size_t>(lump.size) / sizeof(T);
  }

  const BspMipTexture *MipTexture(uint32_t index) const {
    if (header.mipTextures.size < static_cast<int32_t>(sizeof(int32_t))) {
      throw std::runtime_error("invalid mip texture lump");
    }
    const int32_t count = AtRelative<int32_t>(header.mipTextures, 0);
    if (count < 0 || index >= static_cast<uint32_t>(count)) {
      throw std::runtime_error("mip texture index is out of range");
    }
    const int32_t relative = AtRelative<int32_t>(
        header.mipTextures, sizeof(int32_t) + index * sizeof(int32_t));
    if (relative < 0) {
      return nullptr; // Texture lives in an external WAD.
    }
    ValidateSubRange(header.mipTextures, relative, sizeof(BspMipTexture),
                     "invalid mip texture header");
    return reinterpret_cast<const BspMipTexture *>(
        bytes.data() + header.mipTextures.offset + relative);
  }

  std::vector<Color> TexturePixels(uint32_t index,
                                   const BspMipTexture &texture) const {
    // ericw-tools can intentionally emit 0x0 placeholders for missing or
    // external textures. Render those with the fallback checker texture.
    if (texture.width == 0 || texture.height == 0) {
      return {};
    }
    if (texture.width > 4096 || texture.height > 4096) {
      throw std::runtime_error("invalid BSP texture dimensions");
    }

    const int32_t count = AtRelative<int32_t>(header.mipTextures, 0);
    (void)count;
    const int32_t textureRelative = AtRelative<int32_t>(
        header.mipTextures, sizeof(int32_t) + index * sizeof(int32_t));
    if (texture.offsets[0] == 0) {
      return {};
    }
    const uint64_t pixelCount =
        static_cast<uint64_t>(texture.width) * texture.height;
    const int64_t pixelRelative = static_cast<int64_t>(textureRelative) +
                                  static_cast<int64_t>(texture.offsets[0]);
    if (pixelRelative < 0 || pixelCount > std::numeric_limits<int32_t>::max()) {
      throw std::runtime_error("invalid BSP texture data");
    }
    ValidateSubRange(header.mipTextures, static_cast<int32_t>(pixelRelative),
                     static_cast<int32_t>(pixelCount),
                     "invalid BSP texture pixels");

    const uint8_t *indices = bytes.data() + header.mipTextures.offset +
                             static_cast<size_t>(pixelRelative);
    std::vector<Color> pixels(static_cast<size_t>(pixelCount));
    for (size_t i = 0; i < pixels.size(); ++i) {
      pixels[i] = QuakeColor(indices[i]);
    }
    return pixels;
  }

  std::string_view EntityText() const {
    const char *text =
        reinterpret_cast<const char *>(bytes.data() + header.entities.offset);
    size_t size = static_cast<size_t>(header.entities.size);
    if (size > 0 && text[size - 1] == '\0') {
      --size;
    }
    return {text, size};
  }

  BspHeader header{};

private:
  template <typename T> T AtRelative(Lump lump, size_t relative) const {
    ValidateSubRange(lump, static_cast<int32_t>(relative), sizeof(T),
                     "invalid BSP lump data");
    T value{};
    std::memcpy(&value, bytes.data() + lump.offset + relative, sizeof(T));
    return value;
  }

  void ValidateRange(int32_t offset, int32_t size, const char *message) const {
    if (offset < 0 || size < 0 ||
        static_cast<uint64_t>(offset) + size >
            static_cast<uint64_t>(bytes.size())) {
      throw std::runtime_error(message);
    }
  }

  void ValidateSubRange(Lump lump, int32_t relative, int32_t size,
                        const char *message) const {
    if (relative < 0 || size < 0 ||
        static_cast<int64_t>(relative) + size > lump.size) {
      throw std::runtime_error(message);
    }
  }

  static Color QuakeColor(uint8_t index) { return kQuakePalette[index]; }

  std::vector<uint8_t> bytes;
};

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
  SpawnData GetPlayerSpawn() const { return playerSpawn; }
  int GetTriangleCount() const { return triangleCount; }
  const std::vector<Entity> &getEntities() const { return entities; }
  void setEntities(std::vector<Entity> val) { entities = val; }
  BspModel GetModel(size_t index) const {
    if (!worldBsp) {
      throw std::runtime_error("BSP level is not loaded");
    }
    return worldBsp->At<BspModel>(worldBsp->header.models, index);
  }

private:
  struct StaticModelInstance {
    size_t modelIndex = 0;
    Vector3 position{};
    float yaw = 0.0f;
    float scale = 1.0f;
  };
  struct DynamicModelInstance {
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
  std::optional<BspData> worldBsp;
  b3BodyId collisionBodyId = b3_nullBodyId;
  b3MeshData *collisionMesh = nullptr;

  std::string sourcePath;
  SpawnData playerSpawn{};
  bool hasPlayerSpawn = false;
  int triangleCount = 0;
};

} // namespace game
