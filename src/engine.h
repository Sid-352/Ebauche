#pragma once

struct EngineState
{
    float GlobalTime;
    float DeltaTime;
    float SphereSizeMultiplier = 0.4f;
    float FileSizeMultiplier = 1.2f;
    bool IsZenModeEnabled;
    float SimulationSpeed = 1.0f;
    float RenderDistance = 100000.0f;
    double PhysicsTimeMs;
    bool ShowGalacticCenter = true;
    bool ShowSelectionPopup = true;
    float BloomIntensity = 1.0f;
    size_t SelectedNodeIndex = (size_t)-1;
};
