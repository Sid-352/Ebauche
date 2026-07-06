#include "engine.h"
#include "logger.h"
#include "manifest_loader.h"
#include "physics.h"
#include "renderer.h"
#include "scanner.h"
#include "ui_overlay.h"
#include <raylib.h>

int main()
{
    InitWindow(1280, 720, "Ebauche");
    SetTargetFPS(60);

    EngineState engineState;
    engineState.DeltaTime = 0.0f;
    engineState.GlobalTime = 0.0f;
    engineState.IsZenModeEnabled = false;
    engineState.SimulationSpeed = 1.0f;
    engineState.RenderDistance = 4000.0f;
    engineState.PhysicsTimeMs = 0.0;

    RenderContext renderContext;
    LOG_INFO("Initializing Renderer...");
    InitializeRenderer(renderContext);

    Graph graph;
    LOG_INFO("Attempting to load graph manifest tools_manifest.bin...");
    if (!LoadGraphManifest("tools_manifest", graph))
    {
        LOG_INFO("Manifest not found. Beginning ScanDirectory on N:/Tools/...");
        ScanDirectory("N:/Tools/", graph);
        LOG_INFO("ScanDirectory complete. Attempting to save binary manifest...");
        SaveGraphManifest("tools_manifest", graph);
    }
    LOG_INFO("Graph setup complete. Nodes: %zu, Edges: %zu", graph.Nodes.size(), graph.Edges.size());

    LOG_INFO("Initializing UI...");
    InitializeUI();
    LOG_INFO("Entering main render loop...");

    while (!WindowShouldClose())
    {
        double frameStartTime = GetTime();
        engineState.DeltaTime = GetFrameTime();
        engineState.GlobalTime += engineState.DeltaTime;

        if (IsKeyPressed(KEY_Z))
        {
            engineState.IsZenModeEnabled = !engineState.IsZenModeEnabled;
        }

        double startTime = GetTime();
        UpdatePhysics(graph, engineState.DeltaTime * engineState.SimulationSpeed);
        engineState.PhysicsTimeMs = (GetTime() - startTime) * 1000.0;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Ray ray = GetMouseRay(GetMousePosition(), renderContext.Camera);
            float closestHit = 999999999.0f;
            size_t newSelection = (size_t)-1;

            for (size_t i = 0; i < graph.Nodes.size(); i++)
            {
                const auto &node = graph.Nodes[i];
                float r = 2.0f;
                RayCollision collision = GetRayCollisionSphere(ray, node.Position, r);
                if (collision.hit && collision.distance < closestHit)
                {
                    closestHit = collision.distance;
                    newSelection = i;
                }
            }

            if (newSelection != (size_t)-1 && newSelection == engineState.SelectedNodeIndex)
                engineState.SelectedNodeIndex = (size_t)-1;
            else if (newSelection != (size_t)-1)
                engineState.SelectedNodeIndex = newSelection;
        }

        UpdateGraphAnimation(graph, engineState.DeltaTime * engineState.SimulationSpeed);
        DrawScene(renderContext, engineState, graph);
        DrawUI(engineState);
        EndDrawing();
    }

    ShutdownUI();
    ShutdownRenderer(renderContext);
    CloseWindow();

    return 0;
}
