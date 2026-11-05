#pragma once

#include "entities.hpp"
#include "quakeBsp.hpp"
#include <iostream>
#include <sstream>
#include <vector>
inline Vector3 ConvertStringToVector3(const std::string &position_string) {
  std::stringstream ss(position_string);
  Vector3 vec;

  // Stream directly into the float fields, automatically handling spaces
  if (ss >> vec.x >> vec.y >> vec.z) {
    return vec;
  }

  // Return a default vector if parsing fails
  std::cerr << "Error: Failed to parse position string: " << position_string
            << std::endl;
  return Vector3{0.0f, 0.0f, 0.0f};
}
inline const Vector3 centerFromQuake(BspModel &bspModel) {
  const Vector3 center = FromQuake(
      bspModel.origin[0] + (bspModel.mins[0] + bspModel.maxs[0]) * 0.5f,
      bspModel.origin[1] + (bspModel.mins[1] + bspModel.maxs[1]) * 0.5f,
      bspModel.origin[2] + (bspModel.mins[2] + bspModel.maxs[2]) * 0.5f);
  return center;
}
inline const Vector3 halfFromQuake(BspModel &bspModel) {
  const Vector3 halfExtents =
      FromQuake((bspModel.maxs[0] - bspModel.mins[0]) * 0.5f,
                (bspModel.maxs[1] - bspModel.mins[1]) * 0.5f,
                (bspModel.maxs[2] - bspModel.mins[2]) * 0.5f);

  return halfExtents;
}
