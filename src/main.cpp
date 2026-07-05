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
