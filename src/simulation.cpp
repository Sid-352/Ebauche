#include "simulation.h"
#include "math_core.h"

void InitializeSimulation(Planet &outPlanet)
{
    outPlanet.OrbitRadius = 10.0f;
    outPlanet.OrbitalAngle = 0.0f;
    outPlanet.SpeedMultiplier = 1.0f;
    outPlanet.Position = {10.0f, 0.0f, 0.0f};
    outPlanet.BodyColor = BLUE;
    outPlanet.Radius = 1.0f;
}

void UpdateSimulation(Planet &planet, const EngineState &state)
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
