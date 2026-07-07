#pragma once

#include "engine.h"
#include "graph.h"
#include <raylib.h>

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
};

void InitializeRenderer(RenderContext &outContext);
void DrawScene(RenderContext &context, EngineState &state, const Graph &graph);
void UpdateGraphAnimation(Graph &graph, float dt);
void ShutdownRenderer(RenderContext &context);
