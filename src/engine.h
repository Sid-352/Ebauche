#pragma once
#include <cstddef>
#include <cstdint>

struct EngineState
{
    float GlobalTime;
    float DeltaTime;
    float SphereSizeMultiplier = 0.32f;
    float FileSizeMultiplier = 0.96f;
    bool IsZenModeEnabled;
    float SimulationSpeed = 0.5f;
    float RenderDistance = 4000.0f;
    double PhysicsTimeMs;
    bool ShowGalacticCenter = true;
    bool ShowSelectionPopup = true;
    float BloomIntensity = 1.0f;
    size_t SelectedNodeIndex = (size_t)-1;
    char SearchQuery[256] = "";
    bool SearchTriggered = false;
    size_t SearchOffset = 0;
    bool SearchReset = false;

    bool StatsCalculated = false;
    size_t TotalFiles = 0;
    size_t TotalDirs = 0;
    uint64_t TotalSize = 0;
    char RootFolderName[256] = "Orbital Drive";
    char SelectedNodeName[256] = "None";
    char SelectedNodeSize[64] = "0 B";
    size_t VisibleObjects = 0;
    bool OpenFile = false;
};
