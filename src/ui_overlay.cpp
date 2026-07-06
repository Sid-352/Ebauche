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
    ImGui::Text("Physics Calc Time: %.2f ms", state.PhysicsTimeMs);

    ImGui::SliderFloat("Simulation Speed", &state.SimulationSpeed, 0.0f, 10.0f);
    ImGui::SliderFloat("Directory Size", &state.SphereSizeMultiplier, 0.1f, 5.0f);
    ImGui::SliderFloat("File Size", &state.FileSizeMultiplier, 0.1f, 5.0f);
    ImGui::SliderFloat("Bloom Intensity", &state.BloomIntensity, 0.0f, 3.0f);
    
    ImGui::SliderFloat("Render Distance", &state.RenderDistance, 100.0f, 10000000.0f, "%.3f",
                       ImGuiSliderFlags_Logarithmic);
    ImGui::Checkbox("Show Galactic Center", &state.ShowGalacticCenter);
    ImGui::Checkbox("Show Selection Popup", &state.ShowSelectionPopup);

    if (ImGui::Button(state.IsZenModeEnabled ? "Disable Zen Mode" : "Enable Zen Mode"))
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
