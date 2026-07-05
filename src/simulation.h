#pragma once

#include "engine.h"
#include <raylib.h>
#include <vector>

struct Planet
{
    float OrbitRadius;
    float OrbitalAngle;
    float SpeedMultiplier;
    Vector3 Position;
    Color BodyColor;
    float Radius;
};

void UpdateSimulation(std::vector<Planet> &planets, const EngineState &state);
