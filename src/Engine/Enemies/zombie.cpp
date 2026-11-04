#include "zombie.hpp"

Zombie::Zombie(b3Vec3 position, b3Vec3 rotation, b3WorldId worldid)
    : position(position), rotation(rotation) {}

void Zombie::Update() {}

void Zombie::CheckPlayerDistance() {}
