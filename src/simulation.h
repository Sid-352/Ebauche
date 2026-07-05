#pragma once

#include "engine.h"
#include <raylib.h>

struct Planet
{
    float OrbitRadius;
    float OrbitalAngle;
    float SpeedMultiplier;
    Vector3 Position;
    Color BodyColor;
    float Radius;
};

void InitializeSimulation(Planet &outPlanet);
void UpdateSimulation(Planet &planet, const EngineState &state);
