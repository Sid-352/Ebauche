#include "renderer.h"
#include <cmath>

void InitializeRenderer(RenderContext &outContext)
{
    outContext.CameraAngleX = 0.0f;
    outContext.CameraAngleY = 0.5f;
    outContext.CameraRadius = 25.0f;

    outContext.Camera = {0};
    outContext.Camera.target = {0.0f, 0.0f, 0.0f};
    outContext.Camera.up = {0.0f, 1.0f, 0.0f};
    outContext.Camera.fovy = 45.0f;
    outContext.Camera.projection = CAMERA_PERSPECTIVE;

    Mesh sphereMesh = GenMeshSphere(1.0f, 16, 16);
    outContext.SphereModel = LoadModelFromMesh(sphereMesh);
}

void DrawScene(RenderContext &context, const Planet &planet)
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Vector2 mouseDelta = GetMouseDelta();
        context.CameraAngleX -= mouseDelta.x * 0.005f;
        context.CameraAngleY += mouseDelta.y * 0.005f;
    }

    context.CameraRadius -= GetMouseWheelMove() * 2.5f;
    if (context.CameraRadius < 1.0f) context.CameraRadius = 1.0f;

    if (context.CameraAngleY > 1.5f) context.CameraAngleY = 1.5f;
    if (context.CameraAngleY < -1.5f) context.CameraAngleY = -1.5f;

    context.Camera.position.x = context.CameraRadius * std::cos(context.CameraAngleY) * std::sin(context.CameraAngleX);
    context.Camera.position.y = context.CameraRadius * std::sin(context.CameraAngleY);
    context.Camera.position.z = context.CameraRadius * std::cos(context.CameraAngleY) * std::cos(context.CameraAngleX);

    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode3D(context.Camera);

    DrawModel(context.SphereModel, {0.0f, 0.0f, 0.0f}, 2.0f, RED);
    DrawCircle3D({0.0f, 0.0f, 0.0f}, planet.OrbitRadius, {1.0f, 0.0f, 0.0f}, 90.0f, DARKGRAY);
    DrawModel(context.SphereModel, planet.Position, planet.Radius, planet.BodyColor);

    EndMode3D();
}

void ShutdownRenderer(RenderContext &context)
{
    UnloadModel(context.SphereModel);
}
