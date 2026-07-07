#pragma once

#include "../world/world.hpp"
#include <box3d/box3d.h>
#include <raylib.h>

class Player {
public:
  Player() = default;
  ~Player();

  void Init(World &world);
  void CreatePhysicsBody();

  void Input();
  void Update();
  void Render();

  void HandlePlayerMovement();
  void HandleCameraLook();
  void HandleMouseInput();

  void PickupPhysicsObject();

  void RenderGun();

  // Getter Setters for player variables
  b3Pos getPosition() const { return b3Body_GetPosition(collider.bodyId); }
  BoxDef *setHeldObject(BoxDef *object) { return heldObject = object; }

  float *getHeldObjectCooldown() { return &cooldownTimer; }

  b3Vec3 getVelocity() const {
    return b3Body_GetLinearVelocity(collider.bodyId);
  }
  Camera3D &getCamera() { return camera; }
  const Camera3D &getCamera() const { return camera; }

private:
  Camera3D camera{};
  World *_world = nullptr;
  BoxDef collider;
  Model gunModel;
  BoxDef *heldObject = nullptr;

  RenderTexture2D gunTarget;

  Camera3D gunCamera = {0};

  // Camera Variables
  float pitch = 0.0f;
  float yaw = 0.0f;
  float sensitivity = 0.003f;

  // Player Movement Variables
  float moveSpeed = 8.0f;
  float groundAccel = 15.0f;
  float airAccel = 3.0f;
  float groundFriction = 8.0f;
  float jumpSpeed = 5.0f;
  bool isGrounded = false;

  // Physics object pickup variables
  float holdDistance = 2.0f;
  float holdCooldown = 0.5f;
  float cooldownTimer = 0.0f;
};
