#pragma once

#include <box3d/box3d.h>
#include <raylib.h>
#include "../world/world.hpp"

class Player
{
public:
    Player() = default;
    ~Player();

    void Init(World& world);
    void CreatePhysicsBody();


    void Input();
    void Update();
    void Render();

    void HandlePlayerMovement();
    void HandleCameraLook();
    void HandleMouseInput();
    
    void RenderGun();



    b3Pos getPosition() const { return b3Body_GetPosition(collider.bodyId); }
    Camera3D& getCamera() { return camera; }
    const Camera3D& getCamera() const { return camera; }

    float pitch = 0.0f;
    float yaw = 0.0f;
    float sensitivity = 0.003f;
    float moveSpeed      = 8.0f;
    float groundAccel    = 15.0f;
    float airAccel       = 3.0f;
    float groundFriction = 8.0f;
    float jumpSpeed      = 5.0f;
    bool isGrounded = false;

    


private:
    
    Camera3D camera{};
    World* _world = nullptr;
    BoxDef collider;
    Model gunModel;

    RenderTexture2D gunTarget;

    Camera3D gunCamera = { 0 };
    
    



};
