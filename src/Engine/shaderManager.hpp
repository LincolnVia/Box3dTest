#include "global.hpp"

#include <cstdio>
#include <raylib.h>
#include <string>
#include <unordered_map>

#pragma once

class ShaderManager {
public:
  void Init() {

    LoadShaderToStorage(
        "lighting",
        TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    LoadShaderToStorage(
        "cubemap",
        TextFormat("resources/shaders/glsl%i/cubemap.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/cubemap.fs", GLSL_VERSION));

    getShader("lighting").locs[SHADER_LOC_MATRIX_MODEL] =
        GetShaderLocation(getShader("lighting"), "matModel");
    getShader("cubemap").locs[SHADER_LOC_VECTOR_VIEW] =
        GetShaderLocation(getShader("cubemap"), "viewPos");

    int ambientLoc = GetShaderLocation(getShader("lighting"), "ambient");
    SetShaderValue(getShader("lighting"), ambientLoc, &ambient,
                   SHADER_UNIFORM_VEC4);
  }
  ~ShaderManager() {

    for (auto &m : shaders) {
      UnloadShader(m.second);
    }
  }

  void LoadShaderToStorage(const std::string &name,
                           const std::string &fileName_vs,
                           const std::string &fileName_fs) {
    Shader shader = LoadShader(fileName_vs.c_str(), fileName_fs.c_str());
    shaders.emplace(name, shader);
    printf("%s Loaded Into Texture Storage\n", name.c_str());
  };

  Shader &getShader(const std::string &name) { return shaders.at(name); }

private:
  std::unordered_map<std::string, Shader> shaders;
  Vector4 ambient = {1.0f, 1.0f, 1.0f, 1.0f};
};
