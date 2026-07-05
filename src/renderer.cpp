#include "renderer.h"
#include <cmath>
#include <iostream>
#include <raymath.h>
#include <rlgl.h>
#include <vector>

struct ColoredPoint
{
    Vector3 Pos;
    Color Col;
    float Size;
    float Angle;
};

static std::vector<ColoredPoint> backgroundStars;

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
    instancingShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(instancingShader, "viewPos");

    outContext.PostProcessingShader = LoadShader(0, TextFormat("resources/shaders/glsl330/bloom.fs"));

    outContext.Target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

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

    if (backgroundStars.empty())
    {
        for (int i = 0; i < 15000; i++)
        {
            Vector3 pos = {(float)GetRandomValue(-3000, 3000), (float)GetRandomValue(-3000, 3000),
                           (float)GetRandomValue(-3000, 3000)};

            int colorType = GetRandomValue(0, 3);
            Color col;
            if (colorType == 0)
                col = {200, 220, 255, 255};
            else if (colorType == 1)
                col = {255, 255, 255, 255};
            else if (colorType == 2)
                col = {255, 240, 200, 255};
            else
                col = {180, 200, 255, 255};

            float size = (float)GetRandomValue(5, 20) / 10.0f;
            backgroundStars.push_back({pos, col, size, 0.0f});
        }
    }
}

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

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 mouseDelta = GetMouseDelta();
        float dYaw = -mouseDelta.x * 0.005f;
        float dPitch = -mouseDelta.y * 0.005f;

        yaw += dYaw;
        pitch += dPitch;

        if (pitch > 1.5f)
            pitch = 1.5f;
        if (pitch < -1.5f)
            pitch = -1.5f;

        Vector3 pos = context.Camera.position;

        float cosY = cosf(dYaw);
        float sinY = sinf(dYaw);
        float nx = pos.x * cosY - pos.z * sinY;
        float nz = pos.x * sinY + pos.z * cosY;
        pos.x = nx;
        pos.z = nz;

        Vector3 rightAxis = {cosf(yaw), 0.0f, -sinf(yaw)};
        pos = Vector3RotateByAxisAngle(pos, rightAxis, dPitch);

        context.Camera.position = pos;
    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
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
    if (IsKeyDown(KEY_A))
        context.Camera.position = Vector3Add(context.Camera.position, Vector3Scale(right, moveSpeed));
    if (IsKeyDown(KEY_D))
        context.Camera.position = Vector3Subtract(context.Camera.position, Vector3Scale(right, moveSpeed));
    if (IsKeyDown(KEY_SPACE))
        context.Camera.position = Vector3Add(context.Camera.position, Vector3Scale(up, moveSpeed));
    if (IsKeyDown(KEY_LEFT_SHIFT))
        context.Camera.position = Vector3Subtract(context.Camera.position, Vector3Scale(up, moveSpeed));

    context.Camera.target = Vector3Add(context.Camera.position, forward);

    Shader instancingShader = context.DirModels[0].materials[0].shader;
    SetShaderValue(instancingShader, instancingShader.locs[SHADER_LOC_VECTOR_VIEW], &context.Camera.position,
                   SHADER_UNIFORM_VEC3);

    if (IsWindowResized())
    {
        UnloadRenderTexture(context.Target);
        context.Target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    }

    BeginTextureMode(context.Target);
    ClearBackground(BLACK);
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

        float intensity = log10f(node.Mass + 1.0f) / 9.0f;
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
            filePoints.push_back({node.Position, ptColor, size, node.SpinAngle});
        }
    }

    if (!backgroundStars.empty())
    {
        Vector3 camRight = right;
        Vector3 camUp = Vector3CrossProduct(forward, camRight);

        rlDisableBackfaceCulling();
        rlBegin(RL_TRIANGLES);
        float wrapSize = 6000.0f;

        Vector3 circleOffsets[8];
        for (int k = 0; k < 8; k++)
        {
            float angle = k * (PI / 4.0f);
            float ca = cosf(angle);
            float sa = sinf(angle);
            circleOffsets[k] = Vector3Add(Vector3Scale(camRight, ca), Vector3Scale(camUp, sa));
        }

        for (const auto &pt : backgroundStars)
        {
            float dx = pt.Pos.x - camPos.x;
            float dy = pt.Pos.y - camPos.y;
            float dz = pt.Pos.z - camPos.z;

            dx -= wrapSize * roundf(dx / wrapSize);
            dy -= wrapSize * roundf(dy / wrapSize);
            dz -= wrapSize * roundf(dz / wrapSize);

            float finalX = camPos.x + dx;
            float finalY = camPos.y + dy;
            float finalZ = camPos.z + dz;

            float s = pt.Size;
            rlColor4ub(pt.Col.r, pt.Col.g, pt.Col.b, pt.Col.a);
            for (int k = 0; k < 8; k++)
            {
                rlVertex3f(finalX, finalY, finalZ);
                rlVertex3f(finalX + circleOffsets[k].x * s, finalY + circleOffsets[k].y * s,
                           finalZ + circleOffsets[k].z * s);
                rlVertex3f(finalX + circleOffsets[(k + 1) % 8].x * s, finalY + circleOffsets[(k + 1) % 8].y * s,
                           finalZ + circleOffsets[(k + 1) % 8].z * s);
            }
        }
        rlEnd();
        rlEnableBackfaceCulling();
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
            float cosA = cosf(pt.Angle);
            float sinA = sinf(pt.Angle);

            Vector3 localRight = Vector3Add(Vector3Scale(camRight, cosA), Vector3Scale(camUp, sinA));
            Vector3 localUp = Vector3Add(Vector3Scale(camRight, -sinA), Vector3Scale(camUp, cosA));

            Vector3 p0_offset = Vector3Add(Vector3Scale(localRight, -s), Vector3Scale(localUp, -s));
            Vector3 p1_offset = Vector3Add(Vector3Scale(localRight, s), Vector3Scale(localUp, -s));
            Vector3 p2_offset = Vector3Add(Vector3Scale(localRight, s), Vector3Scale(localUp, s));
            Vector3 p3_offset = Vector3Add(Vector3Scale(localRight, -s), Vector3Scale(localUp, s));

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
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    float resolution[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(context.PostProcessingShader, GetShaderLocation(context.PostProcessingShader, "resolution"),
                   resolution, SHADER_UNIFORM_VEC2);

    BeginShaderMode(context.PostProcessingShader);
    DrawTextureRec(context.Target.texture,
                   {0.0f, 0.0f, (float)context.Target.texture.width, (float)-context.Target.texture.height},
                   {0.0f, 0.0f}, WHITE);
    EndShaderMode();

    if (state.ShowGalacticCenter && !state.IsZenModeEnabled)
    {
        Vector2 centerScreen = GetWorldToScreen(Vector3{0, 0, 0}, context.Camera);
        float screenW = (float)GetScreenWidth();
        float screenH = (float)GetScreenHeight();

        Vector3 dirToCenter = Vector3Normalize(Vector3Subtract(Vector3{0, 0, 0}, context.Camera.position));
        Vector3 camForward = Vector3Normalize(Vector3Subtract(context.Camera.target, context.Camera.position));
        bool isBehind = Vector3DotProduct(dirToCenter, camForward) < 0.0f;

        if (isBehind)
        {
            centerScreen.x = screenW - centerScreen.x;
            centerScreen.y = screenH - centerScreen.y;
        }

        float pad = 40.0f;
        bool isOffscreen = centerScreen.x < pad || centerScreen.x > screenW - pad || centerScreen.y < pad ||
                           centerScreen.y > screenH - pad || isBehind;

        if (isOffscreen)
        {
            Vector2 clampedPos = centerScreen;
            clampedPos.x = fmaxf(pad, fminf(screenW - pad, clampedPos.x));
            clampedPos.y = fmaxf(pad, fminf(screenH - pad, clampedPos.y));

            Vector2 dir = Vector2Normalize(Vector2Subtract(centerScreen, clampedPos));
            if (isBehind && centerScreen.x > pad && centerScreen.x < screenW - pad && centerScreen.y > pad &&
                centerScreen.y < screenH - pad)
            {
                clampedPos.y = screenH - pad;
                dir = {0.0f, 1.0f};
            }

            Vector2 p1 = Vector2Add(clampedPos, Vector2Scale(dir, 15.0f));
            Vector2 p2 =
                Vector2Add(clampedPos, Vector2Add(Vector2Scale(dir, -10.0f), Vector2Scale({-dir.y, dir.x}, 10.0f)));
            Vector2 p3 =
                Vector2Add(clampedPos, Vector2Add(Vector2Scale(dir, -10.0f), Vector2Scale({dir.y, -dir.x}, 10.0f)));

            DrawTriangle(p1, p2, p3, {255, 200, 0, 200});
            DrawText("Galactic Center", (int)clampedPos.x - 40, (int)clampedPos.y - 30, 10, {255, 200, 0, 200});
        }
        else
        {
            DrawCircleLines((int)centerScreen.x, (int)centerScreen.y, 10.0f, {255, 200, 0, 100});
            DrawText("Galactic Center", (int)centerScreen.x + 15, (int)centerScreen.y - 5, 10, {255, 200, 0, 100});
        }
    }
}

void UpdateGraphAnimation(Graph &graph, float dt)
{
    for (auto &node : graph.Nodes)
    {
        if (node.ParentIndex == (size_t)-1)
            continue;

        node.OrbitAngle += node.OrbitSpeed * dt;
        node.SpinAngle += node.SpinSpeed * dt;

        Vector3 parentPos = graph.Nodes[node.ParentIndex].Position;
        node.Position.x = parentPos.x + cosf(node.OrbitAngle) * node.OrbitRadius;
        node.Position.y = parentPos.y + node.YOffset + sinf(node.OrbitAngle) * node.OrbitTilt * node.OrbitRadius;
        node.Position.z = parentPos.z + sinf(node.OrbitAngle) * node.OrbitRadius;
    }
}

void ShutdownRenderer(RenderContext &context)
{
    UnloadShader(context.PostProcessingShader);
    UnloadRenderTexture(context.Target);

    for (int i = 0; i < 10; i++)
    {
        UnloadModel(context.DirModels[i]);
    }
}
