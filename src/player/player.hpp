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
  bool TryStepUp(b3Vec3 wishMove, b3Vec3 groundPoint, b3Vec3 &vel);

  void PickupPhysicsObject();

  void RenderGun();

  // Getter Setters for player variables
  b3Pos getPosition() const { return b3Body_GetPosition(collider.bodyId); }
  BoxDef *setHeldObject(BoxDef *object) { return heldObject = object; }
  BoxDef *getHeldObject() const { return heldObject; }
  b3Vec3 GetHoldPosition() const {
    const b3CosSin pitchTrig = b3ComputeCosSin(pitch);
    const b3CosSin yawTrig = b3ComputeCosSin(yaw);

    const b3Vec3 forward = {
        pitchTrig.cosine * yawTrig.sine,
        pitchTrig.sine,
        pitchTrig.cosine * yawTrig.cosine,
    };

    const b3Vec3 eye = b3Add(b3ToVec3(b3Body_GetPosition(collider.bodyId)),
                             {0.0f, 1.3f, 0.0f});

    return b3Add(eye, b3MulSV(holdDistance, forward));
  }
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
  float heldObjectGravityScale = 1.0f;
  float heldObjectLinearDamping = 0.0f;
  float heldObjectAngularDamping = 0.0f;

  b3BodyId holdTargetBodyId = b3_nullBodyId;
  b3JointId holdJointId = b3_nullJointId;
  RenderTexture2D gunTarget;

  Camera3D gunCamera = {0};

  // Camera Variables
  float pitch = 0.0f;
  float yaw = 0.0f;
  float sensitivity = 0.0005f;

  // Player Movement Variables
  float moveSpeed = 8.0f;
  float groundAccel = 15.0f;
  float airAccel = 3.0f;
  float groundFriction = 8.0f;
  float jumpSpeed = 3.0f;
  float jumpRayDistance = 2.5;

  float maxStepHeight = 3.55f;
  float stepForwardProbeDistance = 0.35f;
  float stepLowProbeHeight = 0.18f;
  float stepClearance = 0.08f;
  float minStepHeight = 0.04f;
  float stepMinNormalY = 0.65f;
  float maxExternalHorizontalSpeed = 12.0f;
  float maxExternalUpSpeed = 2.0f;
  float maxExternalDownSpeed = 30.0f;

  bool isGrounded = false;

  // Physics object pickup variables.
  float holdDistance = 5.0f;
  float pickupDistance = 15.0f;
  float pickupRadius = 3.75f;
  float holdCooldown = 0.5f;
  float cooldownTimer = 0.0f;
  b3Vec3 holdOffset = {0.0f, 1.6f, 3.0f};

  /// @name Held object restraint tuning
  /// Values for the motor joint that pulls the cube to the invisible kinematic
  /// hold target in front of the camera.
  /// @{
  float heldLinearHertz = 6.0f; ///< Position spring stiffness. Higher snaps to
                                ///< the hold point faster.
  float heldLinearDampingRatio =
      1.15f; ///< Position spring damping. 1 is critical damping; above 1
             ///< reduces bounce.
  float heldMaxSpringForce = 50000.0f; ///< Strongest force the position spring
                                       ///< can apply before giving up.
  float heldAngularHertz = 10.0f; ///< Rotation spring stiffness. Higher stops
                                  ///< held-object spinning faster.
  float heldAngularDampingRatio =
      1.1f; ///< Rotation spring damping. Above 1 keeps rotation from wobbling.
  float heldMaxSpringTorque =
      25000.0f; ///< Strongest torque the rotation spring can apply.
  float heldMaxVelocityForce =
      5000.0f; ///< Linear velocity brake strength for killing flyaway movement.
  float heldMaxVelocityTorque =
      8000.0f; ///< Angular velocity brake strength for killing flyaway spin.
  float heldLinearDampingWhileHeld =
      2.0f; ///< Extra body drag while held, restored when dropped.
  float heldAngularDampingWhileHeld =
      6.0f; ///< Extra spin drag while held, restored when dropped.
  float heldReleaseMaxSpeed =
      6.0f; ///< Maximum linear speed kept when released. Higher allows
            ///< harder throws; lower makes throws more consistent.
  float heldReleaseMaxAngularSpeed =
      18.0f; ///< Maximum spin speed kept when released. Higher allows more
             ///< tumbling; lower keeps thrown cubes calmer.
  /// @}
};
