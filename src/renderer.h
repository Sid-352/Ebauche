#pragma once

#include "engine.h"
#include "simulation.h"
#include <raylib.h>
#include <vector>

struct RenderContext
{
    Camera3D Camera;
    float CameraAngleX;
    float CameraAngleY;
    float CameraRadius;
    Model SphereModel;
};

void InitializeRenderer(RenderContext &outContext);
void DrawScene(RenderContext &context, const std::vector<Planet> &planets);
void ShutdownRenderer(RenderContext &context);
