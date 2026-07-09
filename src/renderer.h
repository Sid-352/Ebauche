#pragma once

#include "engine.h"
#include "graph.h"
#include <raylib.h>

#include <vector>

struct ColoredPoint
{
    Vector3 Position;
    Color Col;
    float Size;
    float Angle;
};

struct RenderContext
{
    Camera3D Camera;
    float CameraSpeed;
    float CameraPitch;
    float CameraYaw;
    Shader InstancingShader;
    Model DirModel;
    Color DirColors[10];
    RenderTexture2D Target;
    Shader PostProcessingShader;
    int locResolution;
    int locBloomIntensity;

    std::vector<ColoredPoint> backgroundStars;
    std::vector<Matrix> dirTransforms[10];
    std::vector<ColoredPoint> filePoints;
};

void InitializeRenderer(RenderContext &outContext);
void UpdateCamera(RenderContext &context, EngineState &state);
void DrawScene(RenderContext &context, EngineState &state, const Graph &graph);
void DrawLoadingScreen(size_t nodesScanned);
void ShutdownRenderer(RenderContext &context);
