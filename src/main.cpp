#include "engine.h"
#include "manifest_loader.h"
#include "math_core.h"
#include "renderer.h"
#include "simulation.h"
#include "ui_overlay.h"
#include <raylib.h>

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Ebauche v0 - Celestial Viewer");
    SetTargetFPS(60);

    EngineState engineState;
    engineState.GlobalTime = 0.0f;
    engineState.DeltaTime = 0.0f;
    engineState.IsZenModeEnabled = false;
    engineState.SimulationSpeed = 1.0f;

    RenderContext renderContext;
    InitializeRenderer(renderContext);

    std::vector<Planet> planets = LoadPlanets("manifest");

    InitializeUI();

    while (!WindowShouldClose())
    {
        engineState.DeltaTime = GetFrameTime();
        engineState.GlobalTime += engineState.DeltaTime;

        if (IsKeyPressed(KEY_Z))
        {
            engineState.IsZenModeEnabled = !engineState.IsZenModeEnabled;
        }

        UpdateSimulation(planets, engineState);

        DrawScene(renderContext, planets);
        DrawUI(engineState);
        EndDrawing();
    }

    ShutdownUI();
    ShutdownRenderer(renderContext);
    CloseWindow();

    return 0;
}
