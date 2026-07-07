#include "../physics.hpp"
#include <box3d/box3d.h>
#include <box3d/math_functions.h>

static void ApplyFriction(b3Vec3& vel, float friction, float dt)
{
    const b3Vec3 horizontalVel = { vel.x, 0.0f, vel.z };
    float speed = b3Length(horizontalVel);
    if (speed < 0.001f) return;

    float drop = speed * friction * dt;
    float newSpeed = speed - drop;
    if (newSpeed < 0.0f) newSpeed = 0.0f;

    const float scale = newSpeed / speed;
    vel.x *= scale;
    vel.z *= scale;
}

static void Accelerate(b3Vec3& vel, b3Vec3 wishDir, float wishSpeed, float accel, float dt)
{
    float currentSpeed = b3Dot(vel, wishDir);
    float addSpeed = wishSpeed - currentSpeed;
    if (addSpeed <= 0.0f) return;

    float accelSpeed = accel * dt * wishSpeed;
    if (accelSpeed > addSpeed) accelSpeed = addSpeed;

    vel = b3MulAdd(vel, accelSpeed, wishDir);
}

