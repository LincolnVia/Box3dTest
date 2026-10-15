// BSP parsing and rendering are adapted from bytesiz3d/quake-level-viewer
// (MIT, Copyright (c) 2023 Abdelrahman Farid).

#include "quakeBsp.hpp"
#include "quakePalette.hpp"

#include "../physics/physics.hpp"

#include <box3d/collision.h>
#include <box3d/types.h>
#include <raymath.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace {
constexpr int32_t kQuakeBspVersion = 29;
constexpr float kQuakeToWorldScale = 0.05f;

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

struct BspModel {
  float mins[3];
  float maxs[3];
  float origin[3];
  int32_t headNodes[4];
  int32_t visibleLeaves;
  int32_t firstFace;
  int32_t faceCount;
};
#pragma pack(pop)

static_assert(sizeof(BspHeader) == 124);
static_assert(sizeof(BspFace) == 20);

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

Vector3 FromQuake(const BspVertex &vertex) {
  return {vertex.y * kQuakeToWorldScale, vertex.z * kQuakeToWorldScale,
          vertex.x * kQuakeToWorldScale};
}

Vector3 FromQuake(float x, float y, float z) {
  return {y * kQuakeToWorldScale, z * kQuakeToWorldScale,
          x * kQuakeToWorldScale};
}

struct RenderVertex {
  Vector3 position;
  Vector2 uv;
};

struct SurfaceGroup {
  uint32_t mipTexture = 0;
  float textureWidth = 0.0f;
  float textureHeight = 0.0f;
  std::vector<RenderVertex> vertices;
};

std::string TextureName(const BspMipTexture *texture, uint32_t index) {
  if (texture == nullptr) {
    return "external_" + std::to_string(index);
  }
  const size_t length = strnlen(texture->name, sizeof(texture->name));
  if (length == 0) {
    return "unnamed_" + std::to_string(index);
  }
  return std::string(texture->name, length);
}

std::filesystem::path FindLooseTexture(std::string_view textureName) {
  const std::filesystem::path textureDirectory = "resources/textures";
  const std::filesystem::path basePath = textureDirectory / textureName;
  for (const char *extension : {"", ".png", ".jpg", ".jpeg"}) {
    const std::filesystem::path candidate = basePath.string() + extension;
    if (FileExists(candidate.string().c_str())) {
      return candidate;
    }
  }

  // Quake's mip texture name field is only 16 bytes. Match a longer loose
  // filename when qbsp had to truncate the material name (for example,
  // "barbage_container" becomes "barbage_contain"). Prefer the shortest
  // match so the base texture wins over named variants such as "_blue".
  if (textureName.size() < sizeof(BspMipTexture{}.name) - 1) {
    return {};
  }

  std::filesystem::path bestMatch;
  std::error_code error;
  for (const std::filesystem::directory_entry &entry :
       std::filesystem::directory_iterator(textureDirectory, error)) {
    if (error || !entry.is_regular_file(error)) {
      continue;
    }
    std::string extension = entry.path().extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char character) {
                     return static_cast<char>(std::tolower(character));
                   });
    if (extension != ".png" && extension != ".jpg" && extension != ".jpeg") {
      continue;
    }
    const std::string stem = entry.path().stem().string();
    if (!stem.starts_with(textureName)) {
      continue;
    }
    if (bestMatch.empty() ||
        stem.size() < bestMatch.stem().string().size() ||
        (stem.size() == bestMatch.stem().string().size() &&
         entry.path().string() < bestMatch.string())) {
      bestMatch = entry.path();
    }
  }
  return bestMatch;
}

Vector2 ResolveTextureSize(const BspMipTexture *texture,
                           std::string_view textureName) {
  if (texture != nullptr && texture->width != 0 && texture->height != 0) {
    return {static_cast<float>(texture->width),
            static_cast<float>(texture->height)};
  }

  const std::filesystem::path candidate = FindLooseTexture(textureName);
  if (!candidate.empty()) {
    Image image = LoadImage(candidate.string().c_str());
    if (IsImageValid(image)) {
      const Vector2 size = {static_cast<float>(image.width),
                            static_cast<float>(image.height)};
      UnloadImage(image);
      return size;
    }
  }
  return {64.0f, 64.0f};
}

Texture2D LoadBspTexture(const BspData &bsp, uint32_t index,
                         const BspMipTexture *texture) {
  if (texture != nullptr) {
    std::vector<Color> pixels = bsp.TexturePixels(index, *texture);
    if (!pixels.empty()) {
      Image image{pixels.data(), static_cast<int>(texture->width),
                  static_cast<int>(texture->height), 1,
                  PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
      Texture2D result = LoadTextureFromImage(image);
      SetTextureFilter(result, TEXTURE_FILTER_POINT);
      SetTextureWrap(result, TEXTURE_WRAP_REPEAT);
      return result;
    }

    const std::filesystem::path textureName(TextureName(texture, index));
    const bool safeRelativePath =
        !textureName.is_absolute() &&
        std::none_of(textureName.begin(), textureName.end(),
                     [](const auto &component) { return component == ".."; });
    if (safeRelativePath) {
      const std::filesystem::path candidate =
          FindLooseTexture(textureName.string());
      if (!candidate.empty()) {
        Texture2D result = LoadTexture(candidate.string().c_str());
        if (IsTextureValid(result)) {
          SetTextureFilter(result, TEXTURE_FILTER_POINT);
          SetTextureWrap(result, TEXTURE_WRAP_REPEAT);
          TraceLog(LOG_INFO, "BSP texture '%s' loaded from %s",
                   textureName.string().c_str(), candidate.string().c_str());
          return result;
        }
      }
    }
  }

  Image fallback = GenImageChecked(64, 64, 8, 8, MAGENTA, BLACK);
  Texture2D result = LoadTextureFromImage(fallback);
  UnloadImage(fallback);
  SetTextureFilter(result, TEXTURE_FILTER_POINT);
  SetTextureWrap(result, TEXTURE_WRAP_REPEAT);
  return result;
}

Model BuildModel(const SurfaceGroup &surface) {
  Mesh mesh{};
  mesh.vertexCount = static_cast<int>(surface.vertices.size());
  mesh.triangleCount = mesh.vertexCount / 3;
  mesh.vertices = static_cast<float *>(MemAlloc(
      static_cast<unsigned int>(mesh.vertexCount * 3 * sizeof(float))));
  mesh.texcoords = static_cast<float *>(MemAlloc(
      static_cast<unsigned int>(mesh.vertexCount * 2 * sizeof(float))));
  mesh.normals = static_cast<float *>(MemAlloc(
      static_cast<unsigned int>(mesh.vertexCount * 3 * sizeof(float))));

  for (int triangle = 0; triangle < mesh.triangleCount; ++triangle) {
    const Vector3 a = surface.vertices[triangle * 3].position;
    const Vector3 b = surface.vertices[triangle * 3 + 1].position;
    const Vector3 c = surface.vertices[triangle * 3 + 2].position;
    const Vector3 normal = Vector3Normalize(
        Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a)));
    for (int corner = 0; corner < 3; ++corner) {
      const int vertexIndex = triangle * 3 + corner;
      const RenderVertex &vertex = surface.vertices[vertexIndex];
      std::memcpy(mesh.vertices + vertexIndex * 3, &vertex.position,
                  sizeof(Vector3));
      std::memcpy(mesh.texcoords + vertexIndex * 2, &vertex.uv,
                  sizeof(Vector2));
      std::memcpy(mesh.normals + vertexIndex * 3, &normal, sizeof(Vector3));
    }
  }

  UploadMesh(&mesh, false);
  return LoadModelFromMesh(mesh);
}

bool ParseVector3(std::string_view text, Vector3 &result) {
  std::array<float, 3> values{};
  std::istringstream stream(std::string{text});
  if (!(stream >> values[0] >> values[1] >> values[2])) {
    return false;
  }
  result = FromQuake(values[0], values[1], values[2]);
  return true;
}

bool FindPlayerSpawn(std::string_view entities, Vector3 &spawn) {
  size_t cursor = 0;
  while ((cursor = entities.find('{', cursor)) != std::string_view::npos) {
    const size_t end = entities.find('}', cursor + 1);
    if (end == std::string_view::npos) {
      break;
    }
    const std::string_view entity = entities.substr(cursor, end - cursor + 1);
    if (entity.find("\"classname\" \"info_player_start\"") !=
            std::string_view::npos ||
        entity.find("\"classname\" \"info_player_deathmatch\"") !=
            std::string_view::npos) {
      const size_t originKey = entity.find("\"origin\"");
      if (originKey != std::string_view::npos) {
        const size_t valueStart = entity.find('"', originKey + 8);
        const size_t valueEnd = valueStart == std::string_view::npos
                                    ? std::string_view::npos
                                    : entity.find('"', valueStart + 1);
        if (valueEnd != std::string_view::npos &&
            ParseVector3(
                entity.substr(valueStart + 1, valueEnd - valueStart - 1),
                spawn)) {
          spawn.y += 1.0f;
          return true;
        }
      }
    }
    cursor = end + 1;
  }
  return false;
}
} // namespace

bool QuakeBspLevel::Load(const std::filesystem::path &path, b3WorldId worldId,
                         std::string &error) {
  bool replacedExistingLevel = false;
  try {
    BspData bsp(path);
    const BspModel &worldModel = bsp.At<BspModel>(bsp.header.models, 0);
    if (worldModel.firstFace < 0 || worldModel.faceCount <= 0 ||
        static_cast<uint64_t>(worldModel.firstFace) + worldModel.faceCount >
            bsp.Count<BspFace>(bsp.header.faces)) {
      throw std::runtime_error("world model has an invalid face range");
    }

    std::unordered_map<std::string, SurfaceGroup> surfaces;
    std::vector<b3Vec3> collisionVertices;
    std::vector<int32_t> collisionIndices;

    for (int32_t offset = 0; offset < worldModel.faceCount; ++offset) {
      const BspFace &face = bsp.At<BspFace>(
          bsp.header.faces, static_cast<size_t>(worldModel.firstFace + offset));
      if (face.edgeCount < 3 ||
          static_cast<int64_t>(face.firstSurfEdge) + face.edgeCount >
              static_cast<int64_t>(bsp.Count<int32_t>(bsp.header.surfEdges)) ||
          face.firstSurfEdge < 0) {
        continue;
      }
      const BspTextureInfo &textureInfo =
          bsp.At<BspTextureInfo>(bsp.header.textureInfo, face.textureInfo);
      const BspMipTexture *mipTexture = bsp.MipTexture(textureInfo.mipTexture);
      const std::string textureName =
          TextureName(mipTexture, textureInfo.mipTexture);
      SurfaceGroup &surface = surfaces[textureName];
      surface.mipTexture = textureInfo.mipTexture;
      if (surface.textureWidth == 0.0f) {
        const Vector2 textureSize =
            ResolveTextureSize(mipTexture, textureName);
        surface.textureWidth = textureSize.x;
        surface.textureHeight = textureSize.y;
      }
      std::vector<RenderVertex> polygon;
      polygon.reserve(face.edgeCount);
      for (uint16_t edgeOffset = 0; edgeOffset < face.edgeCount; ++edgeOffset) {
        const int32_t surfEdge = bsp.At<int32_t>(
            bsp.header.surfEdges,
            static_cast<size_t>(face.firstSurfEdge + edgeOffset));
        const int64_t edgeIndex =
            surfEdge < 0 ? -static_cast<int64_t>(surfEdge) : surfEdge;
        const BspEdge &edge =
            bsp.At<BspEdge>(bsp.header.edges, static_cast<size_t>(edgeIndex));
        const uint16_t vertexIndex = edge.vertices[surfEdge >= 0 ? 0 : 1];
        const BspVertex &quakeVertex =
            bsp.At<BspVertex>(bsp.header.vertices, vertexIndex);
        const Vector3 position = FromQuake(quakeVertex);
        const Vector2 uv = {
            (quakeVertex.x * textureInfo.uAxis[0] +
             quakeVertex.y * textureInfo.uAxis[1] +
             quakeVertex.z * textureInfo.uAxis[2] + textureInfo.uOffset) /
                surface.textureWidth,
            (quakeVertex.x * textureInfo.vAxis[0] +
             quakeVertex.y * textureInfo.vAxis[1] +
             quakeVertex.z * textureInfo.vAxis[2] + textureInfo.vOffset) /
                surface.textureHeight,
        };
        polygon.push_back({position, uv});
      }

      for (size_t i = polygon.size() - 2; i > 0; --i) {
        const RenderVertex triangle[3] = {polygon.back(), polygon[i],
                                          polygon[i - 1]};
        const Vector3 cross = Vector3CrossProduct(
            Vector3Subtract(triangle[1].position, triangle[0].position),
            Vector3Subtract(triangle[2].position, triangle[0].position));
        if (Vector3LengthSqr(cross) < 1.0e-10f) {
          continue;
        }
        surface.vertices.insert(surface.vertices.end(), std::begin(triangle),
                                std::end(triangle));
        for (const RenderVertex &vertex : triangle) {
          const int32_t index = static_cast<int32_t>(collisionVertices.size());
          collisionVertices.push_back(
              {vertex.position.x, vertex.position.y, vertex.position.z});
          collisionIndices.push_back(index);
        }
      }
    }

    if (collisionIndices.empty()) {
      throw std::runtime_error("BSP world model contains no usable triangles");
    }

    Unload();
    replacedExistingLevel = true;
    for (const auto &[name, surface] : surfaces) {
      (void)name;
      if (surface.vertices.empty()) {
        continue;
      }
      const BspMipTexture *texture = bsp.MipTexture(surface.mipTexture);
      textures.push_back(LoadBspTexture(bsp, surface.mipTexture, texture));
      models.push_back(BuildModel(surface));
      models.back().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
          textures.back();
    }

    b3MeshDef meshDef{};
    meshDef.vertices = collisionVertices.data();
    meshDef.vertexCount = static_cast<int>(collisionVertices.size());
    meshDef.indices = collisionIndices.data();
    meshDef.triangleCount = static_cast<int>(collisionIndices.size() / 3);
    meshDef.weldVertices = true;
    meshDef.weldTolerance = 0.0005f;
    meshDef.identifyEdges = true;
    collisionMesh = b3CreateMesh(&meshDef, nullptr, 0);
    if (collisionMesh == nullptr) {
      throw std::runtime_error("Box3D could not create the collision mesh");
    }

    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type = b3_staticBody;
    collisionBodyId = b3CreateBody(worldId, &bodyDef);
    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.baseMaterial.friction = 0.8f;
    shapeDef.filter.categoryBits = CollisionCategory::Static;
    shapeDef.filter.maskBits =
        CollisionCategory::Dynamic | CollisionCategory::Player;
    b3CreateMeshShape(collisionBodyId, &shapeDef, collisionMesh, b3Vec3_one);

    sourcePath = path.string();
    triangleCount = meshDef.triangleCount;
    hasPlayerSpawn = FindPlayerSpawn(bsp.EntityText(), playerSpawn);
    error.clear();
    return true;
  } catch (const std::exception &exception) {
    error = exception.what();
    if (replacedExistingLevel) {
      Unload();
    }
    return false;
  }
}

void QuakeBspLevel::Unload() {
  if (B3_IS_NON_NULL(collisionBodyId)) {
    b3DestroyBody(collisionBodyId);
    collisionBodyId = b3_nullBodyId;
  }
  if (collisionMesh != nullptr) {
    b3DestroyMesh(collisionMesh);
    collisionMesh = nullptr;
  }
  for (Model model : models) {
    // Textures are owned separately, so prevent UnloadModel from touching the
    // shared texture id through material bookkeeping.
    UnloadModel(model);
  }
  for (Texture2D texture : textures) {
    UnloadTexture(texture);
  }
  models.clear();
  textures.clear();
  sourcePath.clear();
  playerSpawn = {};
  hasPlayerSpawn = false;
  triangleCount = 0;
}

void QuakeBspLevel::Draw(Shader shader) {
  for (Model &model : models) {
    model.materials[0].shader = shader;
    DrawModel(model, {}, 1.0f, WHITE);
  }
}
