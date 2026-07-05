#include "renderer.h"
#include <cmath>
#include <iostream>
#include <raymath.h>
#include <rlgl.h>
#include <vector>

void InitializeRenderer(RenderContext &outContext)
{
    outContext.CameraSpeed = 400.0f;

    outContext.Camera = {0};
    outContext.Camera.position = {0.0f, 150.0f, 300.0f};
    outContext.Camera.target = {0.0f, 0.0f, 0.0f};
    outContext.Camera.up = {0.0f, 1.0f, 0.0f};
    outContext.Camera.fovy = 45.0f;
    outContext.Camera.projection = CAMERA_PERSPECTIVE;

    Shader instancingShader = LoadShader(TextFormat("resources/shaders/glsl330/instancing.vs"),
                                         TextFormat("resources/shaders/glsl330/instancing.fs"));
    instancingShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(instancingShader, "instanceTransform");

    for (int i = 0; i < 10; i++)
    {
        outContext.DirModels[i] = LoadModelFromMesh(GenMeshSphere(1.0f, 16, 16));
        outContext.DirModels[i].materials[0].shader = instancingShader;

        float t = i / 9.0f;
        Color bucketColor;

        if (t < 0.5f)
        {
            float localT = t * 2.0f;
            bucketColor.r = (unsigned char)(100 + localT * 155);
            bucketColor.g = (unsigned char)(120 + localT * 30);
            bucketColor.b = (unsigned char)(255 - localT * 205);
            bucketColor.a = 255;
        }
        else
        {
            float localT = (t - 0.5f) * 2.0f;
            bucketColor.r = 255;
            bucketColor.g = (unsigned char)(150 + localT * 105);
            bucketColor.b = (unsigned char)(50 + localT * 205);
            bucketColor.a = 255;
        }

        outContext.DirModels[i].materials[0].maps[MATERIAL_MAP_ALBEDO].color = bucketColor;
    }
}

struct ColoredPoint
{
    Vector3 Pos;
    Color Col;
    float Size;
};

void UpdateCameraFreecam(Camera3D *camera, float dt)
{
    float speed = 400.0f;
    if (IsKeyDown(KEY_LEFT_SHIFT))
        speed *= 5.0f;
}

void DrawScene(RenderContext &context, const EngineState &state, const Graph &graph)
{
    static float pitch = -0.463f;
    static float yaw = 3.14159f;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Vector2 mouseDelta = GetMouseDelta();
        yaw -= mouseDelta.x * 0.005f;
        pitch -= mouseDelta.y * 0.005f;

        if (pitch > 1.5f)
            pitch = 1.5f;
        if (pitch < -1.5f)
            pitch = -1.5f;
    }

    Vector3 forward = {cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw)};
    Vector3 right = {cosf(yaw), 0.0f, -sinf(yaw)};
    Vector3 up = {0.0f, 1.0f, 0.0f};

    context.CameraSpeed += GetMouseWheelMove() * (context.CameraSpeed * 0.2f);
    if (context.CameraSpeed < 1.0f)
        context.CameraSpeed = 1.0f;

    float currentSpeed = context.CameraSpeed;
    if (IsKeyDown(KEY_LEFT_CONTROL))
        currentSpeed *= 10.0f;
    if (IsKeyDown(KEY_LEFT_ALT))
        currentSpeed *= 0.1f;

    float moveSpeed = currentSpeed * GetFrameTime();

    if (IsKeyDown(KEY_W))
        context.Camera.position = Vector3Add(context.Camera.position, Vector3Scale(forward, moveSpeed));
    if (IsKeyDown(KEY_S))
        context.Camera.position = Vector3Subtract(context.Camera.position, Vector3Scale(forward, moveSpeed));
    if (IsKeyDown(KEY_D))
        context.Camera.position = Vector3Add(context.Camera.position, Vector3Scale(right, moveSpeed));
    if (IsKeyDown(KEY_A))
        context.Camera.position = Vector3Subtract(context.Camera.position, Vector3Scale(right, moveSpeed));
    if (IsKeyDown(KEY_SPACE))
        context.Camera.position = Vector3Add(context.Camera.position, Vector3Scale(up, moveSpeed));
    if (IsKeyDown(KEY_LEFT_SHIFT))
        context.Camera.position = Vector3Subtract(context.Camera.position, Vector3Scale(up, moveSpeed));

    context.Camera.target = Vector3Add(context.Camera.position, forward);

    BeginDrawing();
    ClearBackground({5, 5, 10, 255});

    BeginMode3D(context.Camera);

    static std::vector<Matrix> dirTransforms[10];
    static std::vector<ColoredPoint> filePoints;

    for (int i = 0; i < 10; i++)
    {
        dirTransforms[i].clear();
    }
    filePoints.clear();

    Vector3 camPos = context.Camera.position;
    float maxDistSqr = state.RenderDistance * state.RenderDistance;

    for (const auto &node : graph.Nodes)
    {
        Vector3 diff = {node.Position.x - camPos.x, node.Position.y - camPos.y, node.Position.z - camPos.z};
        float distSqr = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

        if (distSqr > maxDistSqr)
            continue;

        float apparentSize = (node.Radius * node.Radius) / (distSqr + 1.0f);
        if (apparentSize < 0.000000001f)
            continue;

        float intensity = log10f(node.Mass + 1.0f) / 9.0f; // 1GB file = max intensity (log10 = 9)
        if (intensity > 1.0f)
            intensity = 1.0f;
        if (intensity < 0.0f)
            intensity = 0.0f;

        if (node.IsDirectory)
        {
            float r = node.Radius * state.SphereSizeMultiplier;
            Matrix transform = MatrixMultiply(MatrixScale(r, r, r),
                                              MatrixTranslate(node.Position.x, node.Position.y, node.Position.z));

            int bucketIndex = (int)(intensity * 9.99f);
            dirTransforms[bucketIndex].push_back(transform);
        }
        else
        {
            Color ptColor;
            // More striking cosmic color gradient
            float t = intensity;
            if (t < 0.25f)
            {
                float l = t * 4.0f;
                ptColor = {0, (unsigned char)(l * 120), (unsigned char)(100 + l * 155), 200};
            }
            else if (t < 0.50f)
            {
                float l = (t - 0.25f) * 4.0f;
                ptColor = {0, (unsigned char)(120 + l * 135), 255, 200};
            }
            else if (t < 0.75f)
            {
                float l = (t - 0.50f) * 4.0f;
                ptColor = {(unsigned char)(l * 255), 255, (unsigned char)((1.0f - l) * 255), 200};
            }
            else
            {
                float l = (t - 0.75f) * 4.0f;
                ptColor = {255, (unsigned char)((1.0f - l) * 200 + 55), 0, 200};
            }

            float size = 0.3f + (intensity * state.FileSizeMultiplier);
            filePoints.push_back({node.Position, ptColor, size});
        }
    }

    for (int i = 0; i < 10; i++)
    {
        if (!dirTransforms[i].empty())
        {
            DrawMeshInstanced(context.DirModels[i].meshes[0], context.DirModels[i].materials[0],
                              dirTransforms[i].data(), (int)dirTransforms[i].size());
        }
    }

    if (!filePoints.empty())
    {
        Vector3 camRight = right;
        Vector3 camUp = Vector3CrossProduct(forward, camRight);

        rlDisableBackfaceCulling();
        rlBegin(RL_QUADS);
        for (const auto &pt : filePoints)
        {
            float s = pt.Size;
            Vector3 p0_offset = Vector3Add(Vector3Scale(camRight, -s), Vector3Scale(camUp, -s));
            Vector3 p1_offset = Vector3Add(Vector3Scale(camRight, s), Vector3Scale(camUp, -s));
            Vector3 p2_offset = Vector3Add(Vector3Scale(camRight, s), Vector3Scale(camUp, s));
            Vector3 p3_offset = Vector3Add(Vector3Scale(camRight, -s), Vector3Scale(camUp, s));

            rlColor4ub(pt.Col.r, pt.Col.g, pt.Col.b, pt.Col.a);
            rlVertex3f(pt.Pos.x + p0_offset.x, pt.Pos.y + p0_offset.y, pt.Pos.z + p0_offset.z);
            rlVertex3f(pt.Pos.x + p1_offset.x, pt.Pos.y + p1_offset.y, pt.Pos.z + p1_offset.z);
            rlVertex3f(pt.Pos.x + p2_offset.x, pt.Pos.y + p2_offset.y, pt.Pos.z + p2_offset.z);
            rlVertex3f(pt.Pos.x + p3_offset.x, pt.Pos.y + p3_offset.y, pt.Pos.z + p3_offset.z);
        }
        rlEnd();
        rlEnableBackfaceCulling();
    }

    EndMode3D();
}

void UpdateGraphAnimation(Graph &graph, float dt)
{
    for (auto &node : graph.Nodes)
    {
        if (node.ParentIndex == (size_t)-1)
            continue;

        node.OrbitAngle += node.OrbitSpeed * dt;

        Vector3 parentPos = graph.Nodes[node.ParentIndex].Position;
        node.Position.x = parentPos.x + cosf(node.OrbitAngle) * node.OrbitRadius;
        node.Position.y = parentPos.y + node.YOffset + sinf(node.OrbitAngle) * node.OrbitTilt * node.OrbitRadius;
        node.Position.z = parentPos.z + sinf(node.OrbitAngle) * node.OrbitRadius;
    }
}

void ShutdownRenderer(RenderContext &context)
{
    for (int i = 0; i < 10; i++)
    {
        UnloadModel(context.DirModels[i]);
    }
}
