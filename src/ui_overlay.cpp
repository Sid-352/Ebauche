#include "ui_overlay.h"
#include "imgui.h"
#include "rlImGui.h"

void InitializeUI()
{
    rlImGuiSetup(true);
}

void DrawUI(EngineState &state)
{
    if (state.IsZenModeEnabled)
    {
        return;
    }

    rlImGuiBegin();

    ImGui::Begin("Diagnostic Overlay");

    ImGui::Text("DeltaTime: %f", state.DeltaTime);
    ImGui::Text("GlobalTime: %f", state.GlobalTime);

    ImGui::SliderFloat("Simulation Speed", &state.SimulationSpeed, 0.0f, 10.0f);

    if (ImGui::Button("Enable Zen Mode"))
    {
        state.IsZenModeEnabled = true;
    }

    ImGui::End();

    rlImGuiEnd();
}

void ShutdownUI()
{
    rlImGuiShutdown();
}
