#include "player.hpp"
#include "../Engine/global.hpp"
#include "../graphics.hpp"
#include "../utils.hpp"
#include "movement.hpp"
#include <box3d/box3d.h>
#include <box3d/types.h>
#include <raylib.h>

void Player::CreatePhysicsBody() {
  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.type = b3_dynamicBody;
  bodyDef.position = b3ToPos({0.0f, 2.0f, 0.0f});
  bodyDef.motionLocks.angularX = true;
  bodyDef.motionLocks.angularZ = true;

  bodyDef.motionLocks.angularY = false;
  collider.bodyId = b3CreateBody(_world->GetWorldId(), &bodyDef);

  collider.halfExtents = {1.0f, 2.5f, 1.0f};
  collider.hull = b3MakeBoxHull(collider.halfExtents.x, collider.halfExtents.y,
                                collider.halfExtents.z);

  b3ShapeDef shapeDef = b3DefaultShapeDef();
  shapeDef.density = 1000.5f;
  shapeDef.baseMaterial.friction = 0.0f;
  shapeDef.filter.categoryBits = CollisionCategory::Player;
  shapeDef.filter.maskBits = CollisionCategory::World;
  collider.shapeId =
      b3CreateHullShape(collider.bodyId, &shapeDef, &collider.hull.base);
}

void Player::Init(World &world) {
  _world = &world;

  CreatePhysicsBody();

  camera.position = B3PosToVector3(getPosition());
  camera.target = {0.0f, 2.0f, 1.0f};
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  DisableCursor();

  gunModel = LoadModel("resources/models/portalgun.gltf");

  gunTarget = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

  gunCamera.position = {0.0f, 0.0f, 4.0f};
  gunCamera.target = {0.0f, 0.0f, 0.0f};
  gunCamera.up = {0.0f, 1.0f, 0.0f};
  gunCamera.fovy = 45.0f;
  gunCamera.projection = CAMERA_PERSPECTIVE;
}
Player::~Player() { UnloadModel(gunModel); }
void Player::Input() {
  HandleCameraLook();
  PickupPhysicsObject();
  HandlePlayerMovement();
  HandleMouseInput();
}

void Player::Update() {
  Input();
  const b3Vec3 bodyPosition = b3ToVec3(b3Body_GetPosition(collider.bodyId));
  const b3Vec3 cameraPosition = b3Add(bodyPosition, {0.0f, 1.0f, 0.0f});

  const b3CosSin pitchTrig = b3ComputeCosSin(pitch);
  const b3CosSin yawTrig = b3ComputeCosSin(yaw);
  const b3Vec3 view = {pitchTrig.cosine * yawTrig.sine, pitchTrig.sine,
                       pitchTrig.cosine * yawTrig.cosine};

  camera.position = B3Vec3ToVector3(cameraPosition);
  camera.target = B3Vec3ToVector3(b3Add(cameraPosition, view));
}

void Player::RenderGun() {
  BeginTextureMode(gunTarget);
  ClearBackground({0, 0, 0, 0}); // transparent
  BeginMode3D(gunCamera);
  DrawModel(gunModel, {1, -1, 0}, 0.02f, WHITE);
  EndMode3D();
  EndTextureMode();

  // Draw the gun texture over the screen
  Rectangle src = {0.0f, 0.0f, (float)gunTarget.texture.width,
                   -(float)gunTarget.texture.height};
  Rectangle dst = {0.0f, 0.0f, (float)GetScreenWidth(),
                   (float)GetScreenHeight()};
  Vector2 origin = {0.0f, 0.0f};

  DrawTexturePro(gunTarget.texture, src, dst, origin, 0.0f, WHITE);
}

void Player::Render() {}

void Player::HandleCameraLook() {
  Vector2 mouseDelta = GetMouseDelta();

  yaw -= mouseDelta.x * sensitivity;
  pitch -= mouseDelta.y * sensitivity;

  const float maxPitch = 89.0f * B3_DEG_TO_RAD;
  pitch = b3ClampFloat(pitch, -maxPitch, maxPitch);
}

void Player::HandlePlayerMovement() {
  float dt = GetFrameTime();

  const b3Vec3 bodyPos = b3ToVec3(b3Body_GetPosition(collider.bodyId));

  const b3Vec3 feetPos = {bodyPos.x, bodyPos.y - collider.halfExtents.y - 0.05f,
                          bodyPos.z};

  RayFireResult groundRay = FireRay(bodyPos, {0, -1, 0}, *_world,
                                    jumpRayDistance, CollisionCategory::World);
  isGrounded = groundRay.hit.hit;

  const b3CosSin yawTrig = b3ComputeCosSin(yaw);
  const b3Vec3 forward = {yawTrig.sine, 0.0f, yawTrig.cosine};
  const b3Vec3 right = {yawTrig.cosine, 0.0f, -yawTrig.sine};

  b3Vec3 wishMove = b3Vec3_zero;

  if (IsKeyDown(KEY_W))
    wishMove = b3Add(wishMove, forward);
  if (IsKeyDown(KEY_S))
    wishMove = b3Sub(wishMove, forward);
  if (IsKeyDown(KEY_A))
    wishMove = b3Add(wishMove, right);
  if (IsKeyDown(KEY_D))
    wishMove = b3Sub(wishMove, right);

  if (b3LengthSquared(wishMove) > 0.0f)
    wishMove = b3Normalize(wishMove);

  b3Vec3 vel = b3Body_GetLinearVelocity(collider.bodyId);
  const bool wantsMove = b3LengthSquared(wishMove) > 0.0f;

  if (isGrounded) {
    ApplyFriction(vel, groundFriction, dt);

    if (wantsMove)
      Accelerate(vel, wishMove, moveSpeed, groundAccel, dt);

    if (IsKeyPressed(KEY_SPACE)) {
      vel.y = jumpSpeed;
      isGrounded = false;
    }
  } else {
    if (wantsMove)
      Accelerate(vel, wishMove, moveSpeed, airAccel, dt);
  }

  const b3Vec3 horizontalVelocity = {vel.x, 0.0f, vel.z};
  const b3Vec3 clampedHorizontalVelocity =
      ClampVectorLength(horizontalVelocity, maxExternalHorizontalSpeed);
  vel.x = clampedHorizontalVelocity.x;
  vel.z = clampedHorizontalVelocity.z;
  vel.y = b3ClampFloat(vel.y, -maxExternalDownSpeed, maxExternalUpSpeed);

  b3Body_SetLinearVelocity(collider.bodyId, vel);
}

// Refactor for walls instead of companion cube
void Player::HandleMouseInput() {
  RayFireResult ray =
      FireCameraRay(camera, *_world, 100.0f, CollisionCategory::World);

  DrawRayDebug(ray.origin, ray.translation);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (ray.hit.hit) {
      b3ShapeId shapeId = ray.hit.shapeId;
      b3BodyId bodyId = b3Shape_GetBody(shapeId);

      const b3Vec3 hitPoint = b3ToVec3(ray.hit.point);
      if (debugMode) {
        printf("Hit object!\n");
        printf("Hit Point: %.2f %.2f %.2f\n", hitPoint.x, hitPoint.y,
               hitPoint.z);
        printf("Normal: %.2f %.2f %.2f\n", ray.hit.normal.x, ray.hit.normal.y,
               ray.hit.normal.z);
      }
      // Get BoxDef from bodyId
      BoxDef *hitBox = nullptr;

      for (auto &box : _world->GetSceneObjects()) {
        if (B3_ID_EQUALS(box.bodyId, bodyId)) {
          hitBox = &box;
          break;
        }
      }
      if (hitBox && hitBox->name == "companion_cube") {
        printf("Hit companion cube\n");
      }
    } else {
      printf("No hit.\n");
    }
  }
  // void* userData = b3Shape_GetUserData(ray.hit.shapeId);
}

void Player::PickupPhysicsObject() {
  if (IsKeyPressed(KEY_E) && heldObject != nullptr) {
    // Dropping removes the constraint and helper body, then restores the
    // object's normal physics tuning. Velocity is kept, but capped so the
    // cube can be thrown without gaining runaway speed from the hold joint.
    if (B3_IS_NON_NULL(holdJointId)) {
      b3DestroyJoint(holdJointId, true);
      holdJointId = b3_nullJointId;
    }

    if (B3_IS_NON_NULL(holdTargetBodyId)) {
      b3DestroyBody(holdTargetBodyId);
      holdTargetBodyId = b3_nullBodyId;
    }

    b3Body_SetGravityScale(heldObject->bodyId, heldObjectGravityScale);
    b3Body_SetLinearDamping(heldObject->bodyId, heldObjectLinearDamping);
    b3Body_SetAngularDamping(heldObject->bodyId, heldObjectAngularDamping);
    b3Body_SetLinearVelocity(
        heldObject->bodyId,
        ClampVectorLength(b3Body_GetLinearVelocity(heldObject->bodyId),
                          heldReleaseMaxSpeed));
    b3Body_SetAngularVelocity(
        heldObject->bodyId,
        ClampVectorLength(b3Body_GetAngularVelocity(heldObject->bodyId),
                          heldReleaseMaxAngularSpeed));

    heldObject = nullptr;
    return;
  }
  if (IsKeyPressed(KEY_E) && heldObject == nullptr && cooldownTimer <= 0.0f) {
    RayFireResult ray =
        FireCameraSphereRay(camera, *_world, pickupDistance, pickupRadius,
                            CollisionCategory::Dynamic);

    if (ray.hit.hit) {
      b3ShapeId shapeId = ray.hit.shapeId;
      b3BodyId bodyId = b3Shape_GetBody(shapeId);

      BoxDef *hitBox = nullptr;
      // simplify this by using a function in world to get the boxdef from
      // bodyid
      for (auto &box : _world->GetSceneObjects()) {
        if (B3_ID_EQUALS(box.bodyId, bodyId)) {
          hitBox = &box;
          break;
        }
      }

      if (hitBox && hitBox->name == "companion_cube") {
        printf("Holding companion cube\n");
        setHeldObject(hitBox);

        // Preserve the cube's normal physics settings. While held, gravity and
        // low damping fight the hold joint and make the cube sag or slingshot.
        heldObjectGravityScale = b3Body_GetGravityScale(heldObject->bodyId);
        heldObjectLinearDamping = b3Body_GetLinearDamping(heldObject->bodyId);
        heldObjectAngularDamping = b3Body_GetAngularDamping(heldObject->bodyId);

        b3Body_SetGravityScale(heldObject->bodyId, 0.0f);
        b3Body_SetLinearDamping(heldObject->bodyId, heldLinearDampingWhileHeld);
        b3Body_SetAngularDamping(heldObject->bodyId,
                                 heldAngularDampingWhileHeld);
        b3Body_SetLinearVelocity(heldObject->bodyId, b3Vec3_zero);
        b3Body_SetAngularVelocity(heldObject->bodyId, b3Vec3_zero);

        // This invisible kinematic body is the point the cube follows. Moving a
        // target body lets Box3D solve collisions instead of teleporting the
        // cube.
        b3BodyDef targetDef = b3DefaultBodyDef();
        targetDef.type = b3_kinematicBody;
        targetDef.position = b3ToPos(GetHoldPosition());
        targetDef.enableSleep = false;
        holdTargetBodyId = b3CreateBody(_world->GetWorldId(), &targetDef);

        // Motor joint:
        // body A is the kinematic hold target, body B is the cube.
        // localFrameA/B identity means the cube center tries to match the
        // target body origin.
        b3MotorJointDef jointDef = b3DefaultMotorJointDef();
        jointDef.base.bodyIdA = holdTargetBodyId;
        jointDef.base.bodyIdB = heldObject->bodyId;
        jointDef.base.localFrameA = b3Transform_identity;
        jointDef.base.localFrameB = b3Transform_identity;

        // Hertz is spring stiffness: higher means snappier, but too high can
        // jitter. Damping > 1 is slightly overdamped so it returns without
        // bouncing around the hold point.
        jointDef.linearHertz = heldLinearHertz;
        jointDef.linearDampingRatio = heldLinearDampingRatio;
        jointDef.maxSpringForce = heldMaxSpringForce;

        // Angular spring keeps the cube from tumbling wildly while held.
        jointDef.angularHertz = heldAngularHertz;
        jointDef.angularDampingRatio = heldAngularDampingRatio;
        jointDef.maxSpringTorque = heldMaxSpringTorque;

        // Zero desired relative velocity with a force limit acts like a brake:
        // it removes flyaway energy without making the cube completely rigid.
        jointDef.linearVelocity = b3Vec3_zero;
        jointDef.angularVelocity = b3Vec3_zero;
        jointDef.maxVelocityForce = heldMaxVelocityForce;
        jointDef.maxVelocityTorque = heldMaxVelocityTorque;

        holdJointId = b3CreateMotorJoint(_world->GetWorldId(), &jointDef);
        b3Body_SetAwake(heldObject->bodyId, true);

        cooldownTimer = holdCooldown; // Reset the cooldown timer
      }
    }
  }
  // If the player is holding an object, update its position to stay in front of
  // the player
  if (heldObject != nullptr) {
    if (heldObject != nullptr && B3_IS_NON_NULL(holdTargetBodyId)) {
      const b3Vec3 holdPosition = GetHoldPosition();

      // Drive the kinematic target toward the camera hold point each frame.
      // The motor joint then pulls the cube to that target through the solver.
      b3Body_SetTargetTransform(holdTargetBodyId,
                                {b3ToPos(holdPosition), b3Quat_identity},
                                GetFrameTime(), true);
    }
  }
  // Update the cooldown timer
  if (cooldownTimer > 0.0f && heldObject == nullptr) {
    cooldownTimer -= GetFrameTime();
  }
}
