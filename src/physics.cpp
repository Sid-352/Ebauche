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
        float x = node.Position.x * c - node.Position.z * s;
        float z = node.Position.x * s + node.Position.z * c;
        node.Position.x = x;
        node.Position.z = z;
    }
}
