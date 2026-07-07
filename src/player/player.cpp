#include "player.hpp"
#include "../graphics.hpp"
#include <box3d/box3d.h>
#include <box3d/types.h>
#include <raylib.h>
#include "../utils.hpp"
#include "movement.hpp"
#include "../utils.hpp"

void Player::CreatePhysicsBody()
{
    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type = b3_dynamicBody;
    bodyDef.position = b3ToPos({ 0.0f, 2.0f, 0.0f });
    bodyDef.motionLocks.angularX = true;
    bodyDef.motionLocks.angularZ = true;

    
    bodyDef.motionLocks.angularY = false;
    collider.bodyId = b3CreateBody(_world->GetWorldId(), &bodyDef);

    collider.halfExtents = { 1.0f, 2.5f, 1.0f };
    collider.hull = b3MakeBoxHull(collider.halfExtents.x,collider.halfExtents.y,collider.halfExtents.z);

    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.density = 1.5f;
    shapeDef.baseMaterial.friction = 0.0f;
    shapeDef.filter.categoryBits = CollisionCategory::Player;
    shapeDef.filter.maskBits = CollisionCategory::World;
    collider.shapeId = b3CreateHullShape(collider.bodyId, &shapeDef, &collider.hull.base);
}

void Player::Init(World& world)
{
    _world = &world;

    CreatePhysicsBody();

    camera.position = B3PosToVector3(getPosition());
    camera.target = { 0.0f, 2.0f, 1.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();


    gunModel = LoadModel("resources/models/portalgun.gltf");
    


    gunTarget = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());


    gunCamera.position = { 0.0f, 0.0f, 4.0f };
    gunCamera.target   = { 0.0f, 0.0f, 0.0f };
    gunCamera.up       = { 0.0f, 1.0f, 0.0f };
    gunCamera.fovy     = 45.0f;
    gunCamera.projection = CAMERA_PERSPECTIVE;


}
Player::~Player()
{
    UnloadModel(gunModel);
}
void Player::Input()
{
    HandleCameraLook();
    HandlePlayerMovement();
    HandleMouseInput();
}

void Player::Update()
{
    Input();
    const b3Vec3 bodyPosition = b3ToVec3(b3Body_GetPosition(collider.bodyId));
    const b3Vec3 cameraPosition = b3Add(bodyPosition, { 0.0f, 1.0f, 0.0f });

    const b3CosSin pitchTrig = b3ComputeCosSin(pitch);
    const b3CosSin yawTrig = b3ComputeCosSin(yaw);
    const b3Vec3 view = {
        pitchTrig.cosine * yawTrig.sine,
        pitchTrig.sine,
        pitchTrig.cosine * yawTrig.cosine
    };

    camera.position = B3Vec3ToVector3(cameraPosition);
    camera.target = B3Vec3ToVector3(b3Add(cameraPosition, view));

    
}

void Player::RenderGun()
{
    BeginTextureMode(gunTarget);
    ClearBackground({ 0, 0, 0, 0 });   // transparent
    BeginMode3D(gunCamera);
    DrawModel(gunModel, {1,-1,0}, 0.02f, WHITE);
    EndMode3D();
    EndTextureMode();

    // Draw the gun texture over the screen
    Rectangle src = { 0.0f, 0.0f, (float)gunTarget.texture.width, -(float)gunTarget.texture.height };
    Rectangle dst = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
    Vector2 origin = { 0.0f, 0.0f };

    DrawTexturePro(gunTarget.texture, src, dst, origin, 0.0f, WHITE);
}

void Player::Render()
{
    
}



void Player::HandleCameraLook()
{
    Vector2 mouseDelta = GetMouseDelta();

    yaw -= mouseDelta.x * sensitivity;
    pitch -= mouseDelta.y * sensitivity;

    const float maxPitch = 89.0f * B3_DEG_TO_RAD;
    pitch = b3ClampFloat(pitch, -maxPitch, maxPitch);
}

void Player::HandlePlayerMovement()
{
    float dt = GetFrameTime();

    

    const b3Vec3 bodyPos = b3ToVec3(b3Body_GetPosition(collider.bodyId));

    const b3Vec3 feetPos = {
        bodyPos.x,
        bodyPos.y - collider.halfExtents.y - 0.05f,
        bodyPos.z
    };

    RayFireResult groundRay = FireRay(bodyPos, {0,-1,0}, *_world, 3, CollisionCategory::World);
    isGrounded = groundRay.hit.hit;

    const b3CosSin yawTrig = b3ComputeCosSin(yaw);
    const b3Vec3 forward = { yawTrig.sine, 0.0f, yawTrig.cosine };
    const b3Vec3 right   = { yawTrig.cosine, 0.0f, -yawTrig.sine };

    b3Vec3 wishMove = b3Vec3_zero;

    if (IsKeyDown(KEY_W)) wishMove = b3Add(wishMove, forward);
    if (IsKeyDown(KEY_S)) wishMove = b3Sub(wishMove, forward);
    if (IsKeyDown(KEY_A)) wishMove = b3Add(wishMove, right);
    if (IsKeyDown(KEY_D)) wishMove = b3Sub(wishMove, right);

    if (b3LengthSquared(wishMove) > 0.0f)
        wishMove = b3Normalize(wishMove);

    b3Vec3 vel = b3Body_GetLinearVelocity(collider.bodyId);
    const bool wantsMove = b3LengthSquared(wishMove) > 0.0f;

    if (isGrounded)
    {
        ApplyFriction(vel, groundFriction, dt);

        if (wantsMove)
            Accelerate(vel, wishMove, moveSpeed, groundAccel, dt);

        if (IsKeyPressed(KEY_SPACE))
        {
            vel.y = jumpSpeed;
            isGrounded = false;
        }
    }
    else
    {
        if (wantsMove)
            Accelerate(vel, wishMove, moveSpeed, airAccel, dt);
    }

    b3Body_SetLinearVelocity(collider.bodyId, vel);
}

void Player::HandleMouseInput()
{
    RayFireResult ray = FireCameraRay(camera, *_world, 100.0f, CollisionCategory::World);

    DrawRayDebug(ray.origin, ray.translation);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (ray.hit.hit)
        {
            b3ShapeId shapeId = ray.hit.shapeId;
            b3BodyId bodyId = b3Shape_GetBody(shapeId);

            const b3Vec3 hitPoint = b3ToVec3(ray.hit.point);

            printf("Hit object!\n");
            printf("Hit Point: %.2f %.2f %.2f\n", hitPoint.x, hitPoint.y, hitPoint.z);
            printf("Normal: %.2f %.2f %.2f\n", ray.hit.normal.x, ray.hit.normal.y, ray.hit.normal.z);
            
            if (B3_ID_EQUALS(bodyId, _world->GetCompanionCube().bodyId))
            {
                printf("Hit companion cube\n");
            }
        }
        else
        {
            printf("No hit.\n");
        }
    }
    //void* userData = b3Shape_GetUserData(ray.hit.shapeId);
    
    
}
