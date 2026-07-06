#pragma once
#include <cstddef>
#include <cstdint>

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
    char SearchQuery[256] = "";
    bool SearchTriggered = false;
    bool SearchReset = false;

    bool StatsCalculated = false;
    size_t TotalFiles = 0;
    size_t TotalDirs = 0;
    uint64_t TotalSize = 0;
    char RootFolderName[256] = "Orbital Drive";
    char SelectedNodeName[256] = "None";
    char SelectedNodeSize[64] = "0 B";
    char SectorName[256] = "Uncharted Space";
    size_t VisibleObjects = 0;
};
