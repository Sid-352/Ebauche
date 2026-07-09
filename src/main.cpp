#include "engine.h"
#include "logger.h"
#include "manifest_loader.h"
#include "physics.h"
#include "renderer.h"
#include "scanner.h"
#include "ui_overlay.h"
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include <thread>

static bool icontains(const char *text, const std::string &query)
{
    if (query.empty())
        return true;
    const char *t = text;
    while (*t)
    {
        bool match = true;
        for (size_t i = 0; i < query.size(); i++)
        {
            if (t[i] == '\0' || std::tolower((unsigned char)t[i]) != query[i])
            {
                match = false;
                break;
            }
        }
        if (match)
            return true;
        t++;
    }
    return false;
}

int main()
{
    InitWindow(1280, 720, "Ebauche");
    SetTargetFPS(60);

    EngineState engineState;
    engineState.DeltaTime = 0.0f;
    engineState.GlobalTime = 0.0f;
    engineState.IsZenModeEnabled = false;
    engineState.SimulationSpeed = 0.5f;
    engineState.PhysicsTimeMs = 0.0;

    RenderContext renderContext;
    LOG_INFO("Initializing Renderer...");
    InitializeRenderer(renderContext);

    Graph graph;
    std::atomic<size_t> scanNodeCount = 0;
    std::atomic<bool> scanComplete = false;
    std::thread scanThread;

    enum class AppState
    {
        Menu,
        Scanning,
        Simulation
    };

    AppState appState = AppState::Menu;
    StartupOptions startupOptions;

    LOG_INFO("Initializing UI...");
    InitializeUI();
    LOG_INFO("Entering main render loop...");

    bool graphStatsLogged = false;

    while (!WindowShouldClose())
    {
        if (appState == AppState::Menu)
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawStartupMenu(startupOptions);
            EndDrawing();

            if (startupOptions.IsReady)
            {
                if (startupOptions.IsLoadMode)
                {
                    LOG_INFO("Attempting to load graph manifest %s...", startupOptions.ManifestName);
                    if (LoadGraphManifest(startupOptions.ManifestName, graph))
                    {
                        LOG_INFO("Graph setup complete. Nodes: %zu, Edges: %zu", graph.Nodes.size(), graph.Edges.size());
                        scanComplete = true;
                        appState = AppState::Simulation;
                    }
                    else
                    {
                        startupOptions.IsReady = false;
                    }
                }
                else
                {
                    LOG_INFO("Beginning ScanDirectory async on %s...", startupOptions.DirectoryPath);
                    std::string scanDir = startupOptions.DirectoryPath;
                    std::string manifestOut = startupOptions.ManifestName;
                    scanThread = std::thread(
                        [&graph, &scanNodeCount, &scanComplete, scanDir, manifestOut]()
                        {
                            ScanDirectory(scanDir, graph, &scanNodeCount);
                            LOG_INFO("ScanDirectory complete. Attempting to save binary manifest...");
                            SaveGraphManifest(manifestOut, graph);
                            scanComplete = true;
                        });
                    appState = AppState::Scanning;
                }
            }
            continue;
        }
        else if (appState == AppState::Scanning)
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawLoadingScreen(scanNodeCount.load(std::memory_order_relaxed));
            EndDrawing();

            if (scanComplete)
            {
                if (scanThread.joinable())
                {
                    scanThread.join();
                }
                appState = AppState::Simulation;
            }
            continue;
        }

        if (!graphStatsLogged)
        {
            LOG_INFO("Graph setup complete. Nodes: %zu, Edges: %zu", graph.Nodes.size(), graph.Edges.size());
            graphStatsLogged = true;
        }
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

            float maxDistSqr = engineState.RenderDistance * engineState.RenderDistance;
            Vector3 camPos = renderContext.Camera.position;

            for (size_t i = 0; i < graph.Nodes.size(); i++)
            {
                const auto &node = graph.Nodes[i];

                float dx = node.Position.x - camPos.x;
                float dy = node.Position.y - camPos.y;
                float dz = node.Position.z - camPos.z;
                if (dx * dx + dy * dy + dz * dz > maxDistSqr)
                    continue;

                float massLog = log10f((float)node.Mass + 1.0f);
                float intensity = massLog / 10.0f;
                float r = 2.0f;

                if (node.IsDirectory)
                {
                    float safeIntensity = intensity > 1.0f ? 1.0f : intensity;
                    if (massLog <= 4.0f)
                    {
                        r = 3.0f * engineState.SphereSizeMultiplier;
                    }
                    else
                    {
                        float invIntensity = 1.0f - safeIntensity;
                        r = (3.0f + invIntensity * 5.0f) * engineState.SphereSizeMultiplier;
                        if (intensity > 1.1f)
                            r *= 1.5f;
                        if (massLog >= 9.0f)
                        {
                            float extraScale = 1.0f + (massLog - 9.0f) * 2.5f;
                            r *= extraScale;
                        }
                    }
                }
                else
                {
                    float safeT = intensity > 1.0f ? 1.0f : intensity;
                    float size = 0.8f + (safeT * 3.5f * engineState.FileSizeMultiplier);
                    if (massLog >= 9.0f)
                    {
                        float extraScale = 1.0f + (massLog - 9.0f) * 2.5f;
                        size *= extraScale;
                    }
                    r = size;
                }

                r = std::max(r, 2.0f);

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

        if (engineState.SearchTriggered && strlen(engineState.SearchQuery) > 0)
        {
            if (engineState.SearchReset)
            {
                engineState.SearchOffset = 0;
                engineState.SearchReset = false;
            }

            bool found = false;
            std::string queryStr = engineState.SearchQuery;
            std::transform(queryStr.begin(), queryStr.end(), queryStr.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            for (size_t i = 0; i < graph.Nodes.size(); i++)
            {
                size_t idx = (i + engineState.SearchOffset) % graph.Nodes.size();

                if (icontains(graph.Nodes[idx].Name, queryStr))
                {
                    engineState.SelectedNodeIndex = idx;
                    engineState.SearchOffset = idx + 1;
                    found = true;
                    break;
                }
            }

            if (!found && engineState.SearchOffset > 0)
            {
                engineState.SearchOffset = 0;
            }

            engineState.SearchTriggered = false;
        }

        if (!engineState.StatsCalculated)
        {
            for (size_t i = 0; i < graph.Nodes.size(); i++)
            {
                if (graph.Nodes[i].IsDirectory)
                {
                    engineState.TotalDirs++;
                }
                else
                {
                    engineState.TotalFiles++;
                    engineState.TotalSize += graph.Nodes[i].Mass;
                }
            }
            if (!graph.Nodes.empty())
            {
                snprintf(engineState.RootFolderName, sizeof(engineState.RootFolderName), "%s", graph.Nodes[0].Name);
            }
            engineState.StatsCalculated = true;
        }

        static size_t lastSelected = (size_t)-2;
        if (engineState.SelectedNodeIndex != lastSelected)
        {
            lastSelected = engineState.SelectedNodeIndex;
            if (engineState.SelectedNodeIndex != (size_t)-1 && engineState.SelectedNodeIndex < graph.Nodes.size())
            {
                snprintf(engineState.SelectedNodeName, sizeof(engineState.SelectedNodeName), "%s", graph.Nodes[engineState.SelectedNodeIndex].Name);

                double sz = (double)graph.Nodes[engineState.SelectedNodeIndex].Mass;
                if (sz > 1024.0 * 1024.0 * 1024.0)
                    snprintf(engineState.SelectedNodeSize, 64, "%.2f GB", sz / (1024.0 * 1024.0 * 1024.0));
                else if (sz > 1024.0 * 1024.0)
                    snprintf(engineState.SelectedNodeSize, 64, "%.2f MB", sz / (1024.0 * 1024.0));
                else if (sz > 1024.0)
                    snprintf(engineState.SelectedNodeSize, 64, "%.2f KB", sz / 1024.0);
                else
                    snprintf(engineState.SelectedNodeSize, 64, "%.0f B", sz);
            }
            else
            {
                snprintf(engineState.SelectedNodeName, sizeof(engineState.SelectedNodeName), "None");
                snprintf(engineState.SelectedNodeSize, sizeof(engineState.SelectedNodeSize), "-");
            }
        }

        UpdateCamera(renderContext, engineState);
        BeginDrawing();
        DrawScene(renderContext, engineState, graph);
        DrawUI(engineState);
        EndDrawing();
    }

    ShutdownUI();
    ShutdownRenderer(renderContext);
    ShutdownLogger();
    CloseWindow();

    if (scanThread.joinable())
    {
        scanThread.join();
    }

    return 0;
}
