#pragma once

#include <cmath>
#include <raylib.h>

inline Vector3 CalculateOrbitalPosition(float radius, float theta, float yOffset)
{
    Vector3 position;
    position.x = radius * std::cos(theta);
    position.y = yOffset;
    position.z = radius * std::sin(theta);
    return position;
}
