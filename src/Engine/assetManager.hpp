#include <cstdio>
#include <raylib.h>
#include <string>
#include <unordered_map>
#pragma once

class AssetManager {
public:
  void Init() {

    LoadModelToStorage("worldMesh", "resources/scenes/level_01.obj");
    LoadTextureToStorage("ground", "resources/textures/grass.png");
    LoadTextureToStorage("wall", "resources/textures/Dark/texture_01.png");

    LoadTextureToStorage("bounceGel", "resources/textures/bounce.png");

    LoadTextureToStorage("speedGel", "resources/textures/speed.png");

    LoadModelToStorage("companionCube", "resources/models/scene.gltf");
    LoadModelToStorage("storageCube", "resources/models/button.gltf");
  }
  ~AssetManager() {
    for (auto &m : models) {
      UnloadModel(m.second);
    }
    for (auto &m : textures) {
      UnloadTexture(m.second);
    }
  }

  void LoadTextureToStorage(const std::string &name,
                            const std::string &fileName) {
    Texture2D texture = LoadTexture(fileName.c_str());
    textures.emplace(name, texture);
    printf("%s Loaded Into Texture Storage\n", name.c_str());
  };

  void LoadModelToStorage(const std::string &name,
                          const std::string &fileName) {
    Model model = LoadModel(fileName.c_str());
    models.emplace(name, model);
    printf("%s Loaded Into Model Storage\n", name.c_str());
  }
  Texture2D &getTexture(const std::string &name) { return textures.at(name); }
  Model &getModel(const std::string &name) { return models.at(name); }

private:
  std::unordered_map<std::string, Texture2D> textures;
  std::unordered_map<std::string, Model> models;
};
