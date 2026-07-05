#pragma once

#include "engine.h"
#include "graph.h"
#include <raylib.h>

struct RenderContext
{
    Camera3D Camera;
    float CameraAngleX;
    float CameraAngleY;
    float CameraRadius;
    Model SphereModel;
};

void InitializeRenderer(RenderContext &outContext);
void DrawScene(RenderContext &context, const Graph &graph);
void ShutdownRenderer(RenderContext &context);
