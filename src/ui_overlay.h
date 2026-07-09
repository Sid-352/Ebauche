#pragma once

#include "engine.h"

void InitializeUI();
void DrawUI(EngineState &state);
void ShutdownUI();

struct StartupOptions
{
    bool IsReady = false;
    bool IsLoadMode = false;
    char DirectoryPath[1024] = "C:\\";
    char ManifestName[256] = "";
    bool AutoNameManifest = true;
};

void DrawStartupMenu(StartupOptions &options);
