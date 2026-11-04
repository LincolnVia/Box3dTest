// BSP parsing and rendering are adapted from bytesiz3d/quake-level-viewer
// (MIT, Copyright (c) 2023 Abdelrahman Farid).

#include "quakeBsp.hpp"

#include "../physics/physics.hpp"
#include "external/tinyobj_loader_c.h"
#include "quakePalette.hpp"
#include <box3d/collision.h>
#include <box3d/types.h>
#include <raymath.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

constexpr float kQuakeToWorldScale = 0.05f;

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
    if (bestMatch.empty() || stem.size() < bestMatch.stem().string().size() ||
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

using Entity = std::unordered_map<std::string, std::string>;

void SkipWhitespace(std::string_view text, size_t &cursor) {
  while (cursor < text.size() &&
         std::isspace(static_cast<unsigned char>(text[cursor]))) {
    ++cursor;
  }
}

bool ParseQuoted(std::string_view text, size_t &cursor, std::string &result) {
  SkipWhitespace(text, cursor);
  if (cursor >= text.size() || text[cursor] != '"') {
    return false;
  }
  ++cursor;
  result.clear();
  while (cursor < text.size() && text[cursor] != '"') {
    if (text[cursor] == '\\' && cursor + 1 < text.size()) {
      ++cursor;
    }
    result.push_back(text[cursor++]);
  }
  if (cursor >= text.size()) {
    return false;
  }
  ++cursor;
  return true;
}

std::vector<Entity> ParseEntities(std::string_view text) {
  std::vector<Entity> entities;
  size_t cursor = 0;
  while ((cursor = text.find('{', cursor)) != std::string_view::npos) {
    ++cursor;
    Entity entity;
    while (cursor < text.size()) {
      SkipWhitespace(text, cursor);
      if (cursor >= text.size() || text[cursor] == '}') {
        if (cursor < text.size()) {
          ++cursor;
        }
        break;
      }
      std::string key;
      std::string value;
      if (!ParseQuoted(text, cursor, key) ||
          !ParseQuoted(text, cursor, value)) {
        cursor = text.find('}', cursor);
        if (cursor != std::string_view::npos) {
          ++cursor;
        }
        break;
      }
      entity.insert_or_assign(std::move(key), std::move(value));
    }
    if (!entity.empty()) {
      entities.push_back(std::move(entity));
    }
  }
  return entities;
}

bool ParseFloat(std::string_view text, float &result) {
  std::istringstream stream(std::string{text});
  return static_cast<bool>(stream >> result) && std::isfinite(result);
}

bool FindPlayerSpawn(const std::vector<Entity> &entities, SpawnData &spawn) {
  for (const Entity &entity : entities) {
    const auto className = entity.find("classname");
    if (className == entity.end() ||
        (className->second != "info_player_start" &&
         className->second != "info_player_deathmatch")) {
      continue;
    }
    const auto origin = entity.find("origin");
    if (origin != entity.end() &&
        ParseVector3(origin->second, spawn.position)) {
      spawn.position.y += 1.0f;
    }
    const auto angle = entity.find("angle");
    if (angle != entity.end() && ParseFloat(angle->second, spawn.rotation)) {
      return true;
    }
  }

  return false;
}

struct StaticPropDefinition {
  std::filesystem::path path;
  Vector3 position{};
  float yaw = 0.0f;
  float scale = 1.0f;
};

std::filesystem::path ResolveBspModelPath(std::string modelName) {
  std::replace(modelName.begin(), modelName.end(), '\\', '/');
  std::filesystem::path relativePath(modelName);
  if (relativePath.is_absolute()) {
    return {};
  }
  relativePath = relativePath.lexically_normal();
  if (std::any_of(relativePath.begin(), relativePath.end(),
                  [](const auto &component) { return component == ".."; })) {
    return {};
  }

  const std::filesystem::path resourcesPrefix = "resources/models";
  const std::string genericPath = relativePath.generic_string();
  if (genericPath.starts_with("resources/models/")) {
    relativePath = relativePath.lexically_relative(resourcesPrefix);
  } else if (genericPath.starts_with("models/")) {
    relativePath = relativePath.lexically_relative("models");
  }

  std::string extension = relativePath.extension().string();
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](unsigned char character) {
                   return static_cast<char>(std::tolower(character));
                 });
  if (extension != ".glb" && extension != ".gltf") {
    return {};
  }
  return resourcesPrefix / relativePath;
}

std::vector<StaticPropDefinition>
FindStaticProps(const std::vector<Entity> &entities) {
  std::vector<StaticPropDefinition> props;
  for (const Entity &entity : entities) {
    const auto className = entity.find("classname");
    if (className == entity.end() || className->second != "prop_static") {
      continue;
    }
    const auto model = entity.find("model");
    if (model == entity.end() || model->second.empty() ||
        model->second.front() == '*') {
      continue;
    }
    StaticPropDefinition prop;
    prop.path = ResolveBspModelPath(model->second);
    if (prop.path.empty()) {
      TraceLog(LOG_WARNING, "Ignored unsafe or unsupported BSP model: %s",
               model->second.c_str());
      continue;
    }
    const auto origin = entity.find("origin");
    if (origin != entity.end() &&
        !ParseVector3(origin->second, prop.position)) {
      TraceLog(LOG_WARNING, "Ignored BSP model with invalid origin: %s",
               model->second.c_str());
      continue;
    }
    const auto angle = entity.find("angle");
    if (angle != entity.end()) {
      ParseFloat(angle->second, prop.yaw);
    }
    const auto scale = entity.find("scale");
    if (scale != entity.end()) {
      float parsedScale = 1.0f;
      if (ParseFloat(scale->second, parsedScale) && parsedScale > 0.0f) {
        prop.scale = parsedScale;
      }
    }
    props.push_back(std::move(prop));
  }
  return props;
}
bool QuakeBspLevel::Load(const std::filesystem::path &path, b3WorldId worldId,
                         std::string &error) {
  bool replacedExistingLevel = false;
  try {
    BspData bsp(path);
    const std::vector<Entity> _entities = ParseEntities(bsp.EntityText());
    entities = _entities;
    const std::vector<StaticPropDefinition> staticProps =
        FindStaticProps(entities);
    const BspModel &_worldModel = bsp.At<BspModel>(bsp.header.models, 0);
    if (_worldModel.firstFace < 0 || _worldModel.faceCount <= 0 ||
        static_cast<uint64_t>(_worldModel.firstFace) + _worldModel.faceCount >
            bsp.Count<BspFace>(bsp.header.faces)) {
      throw std::runtime_error("world model has an invalid face range");
    }
    std::unordered_map<std::string, SurfaceGroup> surfaces;
    std::vector<b3Vec3> collisionVertices;
    std::vector<int32_t> collisionIndices;

    for (int32_t offset = 0; offset < _worldModel.faceCount; ++offset) {
      const BspFace &face =
          bsp.At<BspFace>(bsp.header.faces,
                          static_cast<size_t>(_worldModel.firstFace + offset));
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
        const Vector2 textureSize = ResolveTextureSize(mipTexture, textureName);
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

    std::unordered_map<std::string, size_t> loadedStaticModels;
    for (const StaticPropDefinition &prop : staticProps) {
      const std::string modelPath = prop.path.generic_string();
      auto loaded = loadedStaticModels.find(modelPath);
      if (loaded == loadedStaticModels.end()) {
        if (!FileExists(modelPath.c_str())) {
          TraceLog(LOG_WARNING, "BSP model does not exist: %s",
                   modelPath.c_str());
          continue;
        }
        Model model = LoadModel(modelPath.c_str());
        if (!IsModelValid(model)) {
          TraceLog(LOG_WARNING, "Could not load BSP model: %s",
                   modelPath.c_str());
          continue;
        }
        const size_t modelIndex = staticModels.size();
        staticModels.push_back(model);
        loaded = loadedStaticModels.emplace(modelPath, modelIndex).first;
        TraceLog(LOG_INFO, "Loaded BSP model: %s", modelPath.c_str());
      }
      staticModelInstances.push_back(
          {loaded->second, prop.position, prop.yaw, prop.scale});
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
    worldBsp = bsp;
    triangleCount = meshDef.triangleCount;
    hasPlayerSpawn = FindPlayerSpawn(_entities, playerSpawn);
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
  for (Model model : staticModels) {
    UnloadModel(model);
  }
  models.clear();
  textures.clear();
  staticModels.clear();
  staticModelInstances.clear();
  worldBsp.reset();
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
  for (Model &model : staticModels) {
    for (int materialIndex = 0; materialIndex < model.materialCount;
         ++materialIndex) {
      model.materials[materialIndex].shader = shader;
    }
  }
  for (const StaticModelInstance &instance : staticModelInstances) {
    DrawModelEx(staticModels[instance.modelIndex], instance.position,
                {0.0f, 1.0f, 0.0f}, instance.yaw,
                {instance.scale, instance.scale, instance.scale}, WHITE);
  }
}
