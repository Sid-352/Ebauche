#include "manifest_loader.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::vector<Planet> ReadBinary(const std::string &path)
{
    std::vector<Planet> planets;
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return planets;

    size_t count = 0;
    file.read(reinterpret_cast<char *>(&count), sizeof(count));
    planets.resize(count);
    file.read(reinterpret_cast<char *>(planets.data()), count * sizeof(Planet));

    return planets;
}

static void WriteBinary(const std::string &path, const std::vector<Planet> &planets)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return;

    size_t count = planets.size();
    file.write(reinterpret_cast<const char *>(&count), sizeof(count));
    file.write(reinterpret_cast<const char *>(planets.data()), count * sizeof(Planet));
}

static std::vector<Planet> ReadJson(const std::string &path)
{
    std::vector<Planet> planets;
    std::ifstream file(path);
    if (!file.is_open()) return planets;

    json j;
    file >> j;

    for (const auto &item : j["planets"])
    {
        Planet p;
        p.OrbitRadius = item["OrbitRadius"];
        p.OrbitalAngle = item["OrbitalAngle"];
        p.SpeedMultiplier = item["SpeedMultiplier"];
        p.Position = {0.0f, 0.0f, 0.0f};

        auto color = item["BodyColor"];
        p.BodyColor = {color[0], color[1], color[2], color[3]};

        p.Radius = item["Radius"];
        planets.push_back(p);
    }
    return planets;
}

std::vector<Planet> LoadPlanets(const std::string &baseName)
{
    std::string binPath = baseName + ".bin";
    std::string jsonPath = baseName + ".json";

    if (std::filesystem::exists(binPath))
    {
        return ReadBinary(binPath);
    }

    if (std::filesystem::exists(jsonPath))
    {
        auto planets = ReadJson(jsonPath);
        WriteBinary(binPath, planets);
        return planets;
    }

    return {};
}
