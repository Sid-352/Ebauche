#pragma once

#include "engine.h"
#include "graph.h"
#include <raylib.h>

struct RenderContext
{
    Camera3D Camera;
    float CameraSpeed;
    Model DirModels[10];
};

void InitializeRenderer(RenderContext &outContext);
void DrawScene(RenderContext &context, const EngineState &state, const Graph &graph);
void UpdateGraphAnimation(Graph &graph, float dt);
void ShutdownRenderer(RenderContext &context);
