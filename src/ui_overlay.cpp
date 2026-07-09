#include "ui_overlay.h"
#include "imgui.h"
#include "rlImGui.h"
#include "tinyfiledialogs.h"
#include <algorithm>
#include <filesystem>
#include <raylib.h>
#include <string>
#include <cstring>
#include <array>

void InitializeUI()
{
    rlImGuiSetup(true);
}

void DrawUI(EngineState &state)
{
    if (state.IsZenModeEnabled)
    {
        rlImGuiBegin();
        ImGuiWindowFlags const zenFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
                                          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoInputs;
        ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 50.0f));
        ImGui::Begin("ZenHUD", nullptr, zenFlags);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 0.8f), "Zen Mode (Press Z to exit)");
        ImGui::End();
        rlImGuiEnd();
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

    ImGui::Separator();
    bool const searchChanged = ImGui::InputText("Search File", state.SearchQuery, sizeof(state.SearchQuery));
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

    ImGuiWindowFlags const hudFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
                                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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

    const double sizeTB = static_cast<double>(state.TotalSize) / (1024.0 * 1024.0 * 1024.0 * 1024.0);
    const double sizeGB = static_cast<double>(state.TotalSize) / (1024.0 * 1024.0 * 1024.0);
    if (sizeTB >= 1.0)
    {
        ImGui::Text("%-16s %.2f TB", "Total Mass:", sizeTB);
    }
    else
    {
        ImGui::Text("%-16s %.2f GB", "Total Mass:", sizeGB);
    }

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
    ImGui::Text("You are here");
    ImGui::PopStyleColor();

    ImGui::Text("%-15s %s", "Sector:", state.SelectedNodeName);
    ImGui::Text("%-15s %s", "Mass:", state.SelectedNodeSize);
    ImGui::Text("%-15s %zu", "Visible:", state.VisibleObjects);
    
    if (state.SelectedNodeIndex != (size_t)-1)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if (ImGui::Button("Open in OS", ImVec2(-1, 30)))
        {
            state.OpenFile = true;
        }
    }

    ImGui::PopStyleColor();
    ImGui::End();

    rlImGuiEnd();
}

void DrawStartupMenu(StartupOptions &options)
{
    rlImGuiBegin();
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(GetScreenWidth()) / 2.0f, static_cast<float>(GetScreenHeight()) / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_Once);

    ImGui::Begin("Startup", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::Text("Welcome to Ebauche. Please select an action:");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::RadioButton("Scan New Directory", !options.IsLoadMode))
    {
        options.IsLoadMode = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Load from Manifest", options.IsLoadMode))
    {
        options.IsLoadMode = true;
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (!options.IsLoadMode)
    {
        ImGui::Text("Directory to Scan:");
        ImGui::SetNextItemWidth(450);
        ImGui::InputText("##dir", options.DirectoryPath, sizeof(options.DirectoryPath));
        ImGui::SameLine();
        if (ImGui::Button("Browse##dir"))
        {
            const char* path = tinyfd_selectFolderDialog("Select Directory to Scan", options.DirectoryPath);
            if (path != nullptr)
            {
                snprintf(options.DirectoryPath, sizeof(options.DirectoryPath), "%s", path);
            }
        }

        ImGui::Spacing();
        ImGui::Checkbox("Auto-name Manifest File", &options.AutoNameManifest);
        
        if (options.AutoNameManifest)
        {
            std::string pathStr = options.DirectoryPath;
            std::replace(pathStr.begin(), pathStr.end(), '\\', '_');
            std::replace(pathStr.begin(), pathStr.end(), ':', '_');
            std::replace(pathStr.begin(), pathStr.end(), '/', '_');
            std::replace(pathStr.begin(), pathStr.end(), ' ', '_');

            while(!pathStr.empty() && pathStr[0] == '_')
            {
                pathStr.erase(0, 1);
            }
            
            std::string autoName = "manifest_" + pathStr;
            snprintf(options.ManifestName, sizeof(options.ManifestName), "%s", autoName.c_str());
        }

        ImGui::Text("Manifest File Name (without .bin):");
        ImGui::SetNextItemWidth(450);
        if (options.AutoNameManifest)
        {
            ImGui::BeginDisabled();
        }
        ImGui::InputText("##manifest", options.ManifestName, sizeof(options.ManifestName));
        if (options.AutoNameManifest)
        {
            ImGui::EndDisabled();
        }
    }
    else
    {
        ImGui::Text("Manifest File to Load:");
        ImGui::SetNextItemWidth(450);
        ImGui::InputText("##manifestLoad", options.ManifestName, sizeof(options.ManifestName));
        ImGui::SameLine();
        if (ImGui::Button("Browse##man"))
        {
            const std::array<const char*, 1> filters = { "*.bin" };
            const char* path = tinyfd_openFileDialog("Select Manifest", "", 1, filters.data(), "Binary Manifest", 0);
            if (path != nullptr)
            {
                std::string fullPath = path;
                if (fullPath.size() > 4 && fullPath.substr(fullPath.size() - 4) == ".bin")
                {
                    fullPath = fullPath.substr(0, fullPath.size() - 4);
                }
                snprintf(options.ManifestName, sizeof(options.ManifestName), "%s", fullPath.c_str());
            }
        }
    }

    ImGui::Spacing();
    
    bool canStart = (strlen(options.DirectoryPath) > 0 && strlen(options.ManifestName) > 0);
    if (options.IsLoadMode)
    {
        canStart = (strlen(options.ManifestName) > 0);
    }

    if (!canStart)
    {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Start", ImVec2(120, 40)))
    {
        options.IsReady = true;
    }
    if (!canStart)
    {
        ImGui::EndDisabled();
    }

    ImGui::End();
    rlImGuiEnd();
}

void ShutdownUI()
{
    rlImGuiShutdown();
}
