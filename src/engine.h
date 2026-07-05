#pragma once

struct EngineState
{
    float GlobalTime;
    float DeltaTime;
    float SphereSizeMultiplier = 0.38f;
    float FileSizeMultiplier = 1.5f;
    bool IsZenModeEnabled;
    float SimulationSpeed;
    float RenderDistance;
    double PhysicsTimeMs;
    bool ShowGalacticCenter = true;
};
