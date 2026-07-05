#include "engine.h"
#include "scanner.h"
#include "manifest_loader.h"
#include "physics.h"
#include "renderer.h"
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

    RenderContext renderContext;
    InitializeRenderer(renderContext);

    Graph graph;
    if (!LoadGraphManifest("abominations", graph))
    {
        ScanDirectory("N:/Projects/Abominations Beyond Comprehension", graph);
        SaveGraphManifest("abominations", graph);
    }

    InitializeUI();

    while (!WindowShouldClose())
    {
        engineState.DeltaTime = GetFrameTime();
        engineState.GlobalTime += engineState.DeltaTime;

        if (IsKeyPressed(KEY_Z))
        {
            engineState.IsZenModeEnabled = !engineState.IsZenModeEnabled;
        }

        UpdatePhysics(graph, engineState.DeltaTime * engineState.SimulationSpeed);

        DrawScene(renderContext, graph);
        DrawUI(engineState);
        EndDrawing();
    }

    ShutdownUI();
    ShutdownRenderer(renderContext);
    CloseWindow();

    return 0;
}
