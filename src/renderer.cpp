#include "renderer.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <raymath.h>
#include <rlgl.h>
#include <sstream>
#include <string>
#include <vector>

static std::string FormatBytes(float bytes)
{
    const char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int suffixIndex = 0;
    float count = bytes;
    while (count >= 1024.0f && suffixIndex < 4)
    {
        count /= 1024.0f;
        suffixIndex++;
    }
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << count << " " << suffixes[suffixIndex];
    return ss.str();
}

void DrawLoadingScreen(size_t nodesScanned)
{
    BeginDrawing();
    ClearBackground({10, 10, 15, 255});

    Vector2 p1 = {100.0f, 100.0f};
    Vector2 p2 = {120.0f, 110.0f};
    DrawText(TextFormat("Scanning Drive... %zu objects found", nodesScanned), 100, 150, 30, LIGHTGRAY);
    DrawCircleV(p1, 4.0f, {255, 200, 0, 255});
    DrawCircleV(p2, 4.0f, {200, 220, 255, 255});
    EndDrawing();
}

void InitializeRenderer(RenderContext &outContext)
{
    outContext.CameraSpeed = 400.0f;

    outContext.Camera = {0};
    outContext.Camera.position = {0.0f, 150.0f, 300.0f};
    outContext.Camera.target = {0.0f, 0.0f, 0.0f};
    outContext.Camera.up = {0.0f, 1.0f, 0.0f};
    outContext.Camera.fovy = 45.0f;
    outContext.Camera.projection = CAMERA_PERSPECTIVE;

    outContext.CameraPitch = -0.463f;
    outContext.CameraYaw = 3.14159f;

    outContext.InstancingShader = LoadShader(TextFormat("resources/shaders/glsl330/instancing.vs"),
                                             TextFormat("resources/shaders/glsl330/instancing.fs"));
    outContext.InstancingShader.locs[SHADER_LOC_MATRIX_MODEL] =
        GetShaderLocationAttrib(outContext.InstancingShader, "instanceTransform");
    outContext.InstancingShader.locs[SHADER_LOC_VECTOR_VIEW] =
        GetShaderLocation(outContext.InstancingShader, "viewPos");

    outContext.PostProcessingShader = LoadShader(0, TextFormat("resources/shaders/glsl330/bloom.fs"));

    outContext.locResolution = GetShaderLocation(outContext.PostProcessingShader, "resolution");
    outContext.locBloomIntensity = GetShaderLocation(outContext.PostProcessingShader, "bloomIntensity");

    outContext.Target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    outContext.DirModel = LoadModelFromMesh(GenMeshSphere(1.0f, 16, 16));
    outContext.DirModel.materials[0].shader = outContext.InstancingShader;

    Color starColors[10] = {{255, 40, 40, 255},   {255, 100, 50, 255},  {255, 150, 50, 255},  {255, 200, 80, 255},
                            {255, 240, 100, 255}, {255, 250, 200, 255}, {255, 255, 255, 255}, {180, 220, 255, 255},
                            {100, 200, 255, 255}, {50, 100, 255, 255}};

    for (int i = 0; i < 10; i++)
    {
        outContext.DirColors[i] = starColors[i];
    }

    if (outContext.backgroundStars.empty())
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
            outContext.backgroundStars.push_back({pos, col, size, 0.0f});
        }
    }
}

void UpdateCamera(RenderContext &context, EngineState &state)
{
    Vector3 forward = {cosf(context.CameraPitch) * sinf(context.CameraYaw), sinf(context.CameraPitch),
                       cosf(context.CameraPitch) * cosf(context.CameraYaw)};
    Vector3 right = {cosf(context.CameraYaw), 0.0f, -sinf(context.CameraYaw)};
    Vector3 up = {0.0f, 1.0f, 0.0f};

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
    {
        Vector2 mouseDelta = GetMouseDelta();
        float panSpeed = context.CameraSpeed * 0.05f;
        Vector3 trueUp = Vector3CrossProduct(forward, right);
        context.Camera.position = Vector3Add(context.Camera.position, Vector3Scale(right, -mouseDelta.x * panSpeed));
        context.Camera.position = Vector3Add(context.Camera.position, Vector3Scale(trueUp, mouseDelta.y * panSpeed));
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 mouseDelta = GetMouseDelta();
        context.CameraYaw -= mouseDelta.x * 0.003f;
        context.CameraPitch -= mouseDelta.y * 0.003f;

        context.CameraPitch = std::clamp(context.CameraPitch, -1.5f, 1.5f);

        forward.x = cosf(context.CameraPitch) * sinf(context.CameraYaw);
        forward.y = sinf(context.CameraPitch);
        forward.z = cosf(context.CameraPitch) * cosf(context.CameraYaw);
        right.x = cosf(context.CameraYaw);
        right.z = -sinf(context.CameraYaw);
    }

    context.CameraSpeed += GetMouseWheelMove() * (context.CameraSpeed * 0.2f);
    context.CameraSpeed = std::max(context.CameraSpeed, 1.0f);

    float currentSpeed = context.CameraSpeed;
    if (IsKeyDown(KEY_LEFT_CONTROL))
        currentSpeed *= 10.0f;
    if (IsKeyDown(KEY_LEFT_ALT))
        currentSpeed *= 0.1f;

    float moveSpeed = currentSpeed * state.DeltaTime;

    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
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
    }

    context.Camera.target = Vector3Add(context.Camera.position, forward);

    SetShaderValue(context.InstancingShader, context.InstancingShader.locs[SHADER_LOC_VECTOR_VIEW],
                   &context.Camera.position, SHADER_UNIFORM_VEC3);
}

void DrawScene(RenderContext &context, EngineState &state, const Graph &graph)
{
    if (IsWindowResized())
    {
        UnloadRenderTexture(context.Target);
        context.Target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    }

    BeginTextureMode(context.Target);
    ClearBackground(BLACK);
    rlSetClipPlanes(1.0, state.RenderDistance + 100000.0);
    BeginMode3D(context.Camera);

    for (int i = 0; i < 10; i++)
    {
        context.dirTransforms[i].clear();
    }
    context.filePoints.clear();

    Vector3 camPos = context.Camera.position;
    float maxDistSqr = state.RenderDistance * state.RenderDistance;
    Vector3 camForward = Vector3Normalize(Vector3Subtract(context.Camera.target, context.Camera.position));

    for (size_t i = 0; i < graph.Nodes.size(); i++)
    {
        const auto &node = graph.Nodes[i];
        Vector3 diff = {node.Position.x - camPos.x, node.Position.y - camPos.y, node.Position.z - camPos.z};

        if (diff.x * camForward.x + diff.y * camForward.y + diff.z * camForward.z < -5000.0f)
            continue;

        float distSqr = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

        if (distSqr > maxDistSqr)
            continue;

        float massLog = log10f(node.Mass + 1.0f);
        float intensity = 0.0f;
        if (massLog > 4.0f)
        {
            intensity = powf((massLog - 4.0f) / 5.0f, 0.85f);
        }
        intensity = std::max(intensity, 0.0f);

        if (node.IsDirectory)
        {
            int bucketIndex = 0;
            float r = 2.0f * state.SphereSizeMultiplier;

            float safeIntensity = std::min(intensity, 1.0f);

            bucketIndex = std::min((int)(safeIntensity * 9.99f), 9);

            if (massLog <= 4.0f)
            {
                r = 3.0f * state.SphereSizeMultiplier;
                bucketIndex = 0;
            }
            else
            {
                float invIntensity = 1.0f - safeIntensity;
                r = (3.0f + invIntensity * 5.0f) * state.SphereSizeMultiplier;

                if (intensity > 1.1f)
                {
                    r *= 1.5f;
                    bucketIndex = 6;
                }

                if (massLog >= 9.0f)
                {
                    float extraScale = 1.0f + (massLog - 9.0f) * 2.5f;
                    r *= extraScale;
                    bucketIndex = 8;
                }
                if (massLog >= 11.0f)
                {
                    bucketIndex = 9;
                }
            }
            if (i == state.SelectedNodeIndex)
                r *= 1.5f;

            Matrix transform = MatrixMultiply(MatrixScale(r, r, r),
                                              MatrixTranslate(node.Position.x, node.Position.y, node.Position.z));

            context.dirTransforms[bucketIndex].push_back(transform);
        }
        else
        {
            float t = intensity;
            float safeT = std::min(intensity, 1.0f);

            float hue = 280.0f - (safeT * 260.0f);
            if (hue < 0.0f)
                hue += 360.0f;

            float saturation = 0.6f + (safeT * 0.4f);
            Color ptColor = ColorFromHSV(hue, saturation, 1.0f);

            float size = 0.8f + (safeT * 3.5f * state.FileSizeMultiplier);

            if (massLog >= 9.0f)
            {
                float extraScale = 1.0f + (massLog - 9.0f) * 2.5f;
                size *= extraScale;

                if (massLog >= 11.0f)
                    ptColor = {200, 230, 255, 255};
                else if (massLog >= 10.0f)
                    ptColor = {255, 255, 100, 255};
                else
                    ptColor = {255, 180, 50, 255};
            }

            if (i == state.SelectedNodeIndex)
            {
                size *= 2.0f;
            }

            context.filePoints.push_back({node.Position, ptColor, size, node.SpinAngle});
        }
    }

    if (!context.backgroundStars.empty())
    {
        Vector3 camRight = {cosf(context.CameraYaw), 0.0f, -sinf(context.CameraYaw)};
        Vector3 camUp = Vector3CrossProduct(camForward, camRight);

        rlDisableDepthMask();
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

        for (const auto &pt : context.backgroundStars)
        {
            rlCheckRenderBatchLimit(24);
            float dx = pt.Position.x - camPos.x;
            float dy = pt.Position.y - camPos.y;
            float dz = pt.Position.z - camPos.z;

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
        rlEnableDepthMask();
    }

    for (int i = 0; i < 10; i++)
    {
        int count = (int)context.dirTransforms[i].size();
        if (count > 0)
        {
            context.DirModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = context.DirColors[i];
            DrawMeshInstanced(context.DirModel.meshes[0], context.DirModel.materials[0],
                              context.dirTransforms[i].data(), count);
        }
    }

    if (!context.filePoints.empty())
    {
        Vector3 camForward = {cosf(context.CameraPitch) * sinf(context.CameraYaw), sinf(context.CameraPitch),
                              cosf(context.CameraPitch) * cosf(context.CameraYaw)};
        Vector3 camRight = {cosf(context.CameraYaw), 0.0f, -sinf(context.CameraYaw)};
        Vector3 camUp = Vector3CrossProduct(camForward, camRight);

        rlDisableDepthMask();
        rlDisableBackfaceCulling();
        rlBegin(RL_QUADS);
        for (const auto &pt : context.filePoints)
        {
            rlCheckRenderBatchLimit(4);
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
            rlVertex3f(pt.Position.x + p0_offset.x, pt.Position.y + p0_offset.y, pt.Position.z + p0_offset.z);
            rlVertex3f(pt.Position.x + p1_offset.x, pt.Position.y + p1_offset.y, pt.Position.z + p1_offset.z);
            rlVertex3f(pt.Position.x + p2_offset.x, pt.Position.y + p2_offset.y, pt.Position.z + p2_offset.z);
            rlVertex3f(pt.Position.x + p3_offset.x, pt.Position.y + p3_offset.y, pt.Position.z + p3_offset.z);
        }
        rlEnd();
        rlEnableBackfaceCulling();
        rlEnableDepthMask();
    }

    EndMode3D();
    EndTextureMode();

    size_t totalVisible = context.filePoints.size();
    for (int i = 0; i < 10; i++)
        totalVisible += context.dirTransforms[i].size();
    state.VisibleObjects = totalVisible;

    ClearBackground(BLACK);

    float resolution[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(context.PostProcessingShader, context.locResolution, resolution, SHADER_UNIFORM_VEC2);

    float bInt = state.BloomIntensity;
    SetShaderValue(context.PostProcessingShader, context.locBloomIntensity, &bInt, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(context.PostProcessingShader);
    DrawTextureRec(context.Target.texture,
                   {0.0f, 0.0f, (float)context.Target.texture.width, (float)-context.Target.texture.height},
                   {0.0f, 0.0f}, WHITE);
    EndShaderMode();

    auto DrawOffscreenTracker = [&](Vector3 targetPos, const char *label, Color colorPrimary, Color colorSecondary)
    {
        float screenW = (float)GetScreenWidth();
        float screenH = (float)GetScreenHeight();
        Vector2 screenPos = GetWorldToScreen(targetPos, context.Camera);

        Vector3 dirToTarget = Vector3Normalize(Vector3Subtract(targetPos, context.Camera.position));
        Vector3 camForward = Vector3Normalize(Vector3Subtract(context.Camera.target, context.Camera.position));
        bool isBehind = Vector3DotProduct(dirToTarget, camForward) < 0.0f;

        if (isBehind)
        {
            screenPos.x = screenW - screenPos.x;
            screenPos.y = screenH - screenPos.y;
        }

        float pad = 40.0f;
        bool isOffscreen = screenPos.x < pad || screenPos.x > screenW - pad || screenPos.y < pad ||
                           screenPos.y > screenH - pad || isBehind;

        if (isOffscreen)
        {
            Vector2 clampedPos = screenPos;
            clampedPos.x = fmaxf(pad, fminf(screenW - pad, clampedPos.x));
            clampedPos.y = fmaxf(pad, fminf(screenH - pad, clampedPos.y));

            Vector2 dir = Vector2Normalize(Vector2Subtract(screenPos, clampedPos));
            if (isBehind && screenPos.x > pad && screenPos.x < screenW - pad && screenPos.y > pad &&
                screenPos.y < screenH - pad)
            {
                clampedPos.y = screenH - pad;
                dir = {0.0f, 1.0f};
            }

            if (label)
                DrawText(label, (int)clampedPos.x - 40, (int)clampedPos.y - 10, 10, colorPrimary);

            Vector2 p1 = {clampedPos.x + dir.x * 20.0f, clampedPos.y + dir.y * 20.0f};
            Vector2 p2 = {clampedPos.x + dir.x * 10.0f - dir.y * 5.0f, clampedPos.y + dir.y * 10.0f + dir.x * 5.0f};
            Vector2 p3 = {clampedPos.x + dir.x * 10.0f + dir.y * 5.0f, clampedPos.y + dir.y * 10.0f - dir.x * 5.0f};
            DrawTriangle(p1, p2, p3, colorSecondary);
            return false;
        }
        return true;
    };

    Color hudColor = {255, 200, 0, 200};
    Color hudColorFaded = {255, 200, 0, 100};

    if (state.ShowGalacticCenter && !state.IsZenModeEnabled)
    {
        if (DrawOffscreenTracker({0.0f, 0.0f, 0.0f}, "Galactic Center", hudColor, hudColorFaded))
        {
            Vector2 centerScreen =
                GetWorldToScreenEx({0.0f, 0.0f, 0.0f}, context.Camera, GetScreenWidth(), GetScreenHeight());
            DrawCircleLines((int)centerScreen.x, (int)centerScreen.y, 10.0f, hudColor);
            DrawText("Galactic Center", (int)centerScreen.x + 15, (int)centerScreen.y - 5, 10, hudColor);
        }
    }

    if (state.SelectedNodeIndex != (size_t)-1 && state.SelectedNodeIndex < graph.Nodes.size())
    {
        const auto &selectedNode = graph.Nodes[state.SelectedNodeIndex];

        if (DrawOffscreenTracker(selectedNode.Position, "Selected", hudColor, hudColorFaded))
        {
            Vector2 selScreen =
                GetWorldToScreenEx(selectedNode.Position, context.Camera, GetScreenWidth(), GetScreenHeight());

            float retSize = 15.0f;
            DrawLine((int)selScreen.x - retSize, (int)selScreen.y - retSize, (int)selScreen.x - retSize + 8,
                     (int)selScreen.y - retSize, hudColor);
            DrawLine((int)selScreen.x - retSize, (int)selScreen.y - retSize, (int)selScreen.x - retSize,
                     (int)selScreen.y - retSize + 8, hudColor);
            DrawLine((int)selScreen.x + retSize, (int)selScreen.y - retSize, (int)selScreen.x + retSize - 8,
                     (int)selScreen.y - retSize, hudColor);
            DrawLine((int)selScreen.x + retSize, (int)selScreen.y - retSize, (int)selScreen.x + retSize,
                     (int)selScreen.y - retSize + 8, hudColor);
            DrawLine((int)selScreen.x - retSize, (int)selScreen.y + retSize, (int)selScreen.x - retSize + 8,
                     (int)selScreen.y + retSize, hudColor);
            DrawLine((int)selScreen.x - retSize, (int)selScreen.y + retSize, (int)selScreen.x - retSize,
                     (int)selScreen.y + retSize - 8, hudColor);
            DrawLine((int)selScreen.x + retSize, (int)selScreen.y + retSize, (int)selScreen.x + retSize - 8,
                     (int)selScreen.y + retSize, hudColor);
            DrawLine((int)selScreen.x + retSize, (int)selScreen.y + retSize, (int)selScreen.x + retSize,
                     (int)selScreen.y + retSize - 8, hudColor);

            if (state.ShowSelectionPopup)
            {
                Vector2 panelPos = {selScreen.x + 40.0f, selScreen.y - 40.0f};
                DrawLineEx(selScreen, panelPos, 2.0f, hudColor);
                DrawCircleV(selScreen, 3.0f, hudColor);

                int nameWidth = MeasureText(selectedNode.Name, 20);
                int pathWidth = MeasureText(selectedNode.Path, 10);
                float panelWidth = fmaxf(450.0f, fmaxf((float)nameWidth + 30.0f, (float)pathWidth + 30.0f));
                float panelHeight = 85.0f;

                DrawRectangle((int)panelPos.x, (int)panelPos.y, (int)panelWidth, (int)panelHeight, {10, 15, 25, 200});
                DrawRectangle((int)panelPos.x, (int)panelPos.y, 4, (int)panelHeight, hudColor);
                DrawRectangleLines((int)panelPos.x, (int)panelPos.y, (int)panelWidth, (int)panelHeight, hudColorFaded);

                int textX = (int)panelPos.x + 15;
                int textY = (int)panelPos.y + 10;

                DrawText(selectedNode.Name, textX, textY, 20, WHITE);

                std::string sizeStr = FormatBytes(selectedNode.Mass);
                if (selectedNode.IsDirectory)
                    sizeStr = "Directory | " + sizeStr;
                else
                    sizeStr = "File | " + sizeStr;

                DrawText(sizeStr.c_str(), textX, textY + 25, 10, {200, 220, 255, 255});
                DrawText(selectedNode.Path, textX, textY + 45, 10, GRAY);
            }
        }
    }
}

void ShutdownRenderer(RenderContext &context)
{
    context.backgroundStars.clear();
    for (int i = 0; i < 10; i++)
        context.dirTransforms[i].clear();
    context.filePoints.clear();

    UnloadShader(context.InstancingShader);
    UnloadShader(context.PostProcessingShader);
    UnloadRenderTexture(context.Target);

    context.DirModel.materials[0].shader = {0};
    UnloadModel(context.DirModel);
}
