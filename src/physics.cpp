#include "physics.h"
#include <cmath>

void UpdatePhysics(Graph &graph, float deltaTime)
{
    float rotationSpeed = 0.05f;
    float angle = deltaTime * rotationSpeed;
    float c = cosf(angle);
    float s = sinf(angle);

    for (auto &node : graph.Nodes)
    {
        if (node.ParentIndex == (size_t)-1)
        {
            float x = node.Position.x * c - node.Position.z * s;
            float z = node.Position.x * s + node.Position.z * c;
            node.Position.x = x;
            node.Position.z = z;
        }
        else
        {
            node.OrbitAngle += node.OrbitSpeed * deltaTime;
            node.SpinAngle += node.SpinSpeed * deltaTime;
            node.RadiusJitterPhase += node.RadiusJitterSpeed * deltaTime;

            float currentRadius = node.OrbitRadius + sinf(node.RadiusJitterPhase) * node.RadiusJitterAmp;

            float localX = cosf(node.OrbitAngle) * currentRadius;
            float localZ = sinf(node.OrbitAngle) * currentRadius * (1.0f - node.Eccentricity);

            float rot = node.OrbitRotation;
            float worldX = localX * cosf(rot) - localZ * sinf(rot);
            float worldZ = localX * sinf(rot) + localZ * cosf(rot);

            Vector3 parentPos = graph.Nodes[node.ParentIndex].Position;
            node.Position.x = parentPos.x + worldX;
            node.Position.y = parentPos.y + node.YOffset + sinf(node.OrbitAngle) * node.OrbitTilt * currentRadius;
            node.Position.z = parentPos.z + worldZ;
        }
    }
}
