#pragma once

struct EngineState
{
    float GlobalTime;
    float DeltaTime;
    float SphereSizeMultiplier = 0.38f;
    float FileSizeMultiplier = 1.5f;
    bool IsZenModeEnabled;
    float SimulationSpeed = 1.0f;
    float RenderDistance = 100000.0f;
    double PhysicsTimeMs;
    bool ShowGalacticCenter = true;
    size_t SelectedNodeIndex = (size_t)-1;
};
