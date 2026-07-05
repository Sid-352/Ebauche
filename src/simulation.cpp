#include "simulation.h"
#include "math_core.h"

void UpdateSimulation(std::vector<Planet> &planets, const EngineState &state)
{
    for (auto &planet : planets)
    {
        planet.OrbitalAngle += state.SimulationSpeed * planet.SpeedMultiplier * state.DeltaTime;

        if (planet.OrbitalAngle > 2.0f * PI)
        {
            planet.OrbitalAngle -= 2.0f * PI;
        }
        else if (planet.OrbitalAngle < 0.0f)
        {
            planet.OrbitalAngle += 2.0f * PI;
        }

        planet.Position = CalculateOrbitalPosition(planet.OrbitRadius, planet.OrbitalAngle, 0.0f);
    }
}
