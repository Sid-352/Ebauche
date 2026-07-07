#include "ui_overlay.h"
#include "imgui.h"
#include "rlImGui.h"
#include <raylib.h>

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

    ImGui::SliderFloat("Render Distance", &state.RenderDistance, 100.0f, 1000000.0f, "%.3f",
                       ImGuiSliderFlags_Logarithmic);
    ImGui::Checkbox("Show Galactic Center", &state.ShowGalacticCenter);
    ImGui::Checkbox("Show Selection Popup", &state.ShowSelectionPopup);

    ImGui::Separator();
    bool searchChanged = ImGui::InputText("Search File", state.SearchQuery, sizeof(state.SearchQuery));
    ImGui::SameLine();
    if (ImGui::Button("Next"))
    {
        state.SearchTriggered = true;
    }
    if (searchChanged)
    {
        state.SearchTriggered = true;
        state.SearchReset = true;
    }
    ImGui::Separator();

    if (ImGui::Button(state.IsZenModeEnabled ? "Disable Zen Mode" : "Enable Zen Mode"))
    {
        state.IsZenModeEnabled = !state.IsZenModeEnabled;
    }

    ImGui::End();

    ImGuiWindowFlags hudFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
                                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;
    ImGui::SetNextWindowPos(ImVec2(GetScreenWidth() - 270.0f, 40.0f));
    ImGui::SetNextWindowSize(ImVec2(260.0f, 400.0f));
    ImGui::Begin("InfoHUD", nullptr, hudFlags);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
    ImGui::Text("Milky Drive");
    ImGui::Text("------------------------");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::Text("%-16s %zu", "Files:", state.TotalFiles);
    ImGui::Text("%-16s %zu", "Directories:", state.TotalDirs);

    double sizeTB = (double)state.TotalSize / (1024.0 * 1024.0 * 1024.0 * 1024.0);
    double sizeGB = (double)state.TotalSize / (1024.0 * 1024.0 * 1024.0);
    if (sizeTB >= 1.0)
        ImGui::Text("%-16s %.2f TB", "Total Mass:", sizeTB);
    else
        ImGui::Text("%-16s %.2f GB", "Total Mass:", sizeGB);

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
    ImGui::Text("You are here");
    ImGui::PopStyleColor();

    ImGui::Text("%-15s %s", "Sector:", state.SelectedNodeName);
    ImGui::Text("%-15s %s", "Mass:", state.SelectedNodeSize);
    ImGui::Text("%-15s %zu", "Visible:", state.VisibleObjects);

    ImGui::PopStyleColor();
    ImGui::End();

    rlImGuiEnd();
}

void ShutdownUI()
{
    rlImGuiShutdown();
}
