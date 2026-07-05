#include "physics.h"
#include <cmath>
#include <raymath.h>

void UpdatePhysics(Graph &graph, float deltaTime)
{
    if (deltaTime <= 0.0f) return;

    float k = 2.0f; 
    float k_squared = k * k;

    // Reset forces
    for (auto &node : graph.Nodes)
    {
        node.Force = {0.0f, 0.0f, 0.0f};
    }

    // Repulsion (O(N^2))
    for (size_t i = 0; i < graph.Nodes.size(); ++i)
    {
        for (size_t j = i + 1; j < graph.Nodes.size(); ++j)
        {
            Vector3 diff = Vector3Subtract(graph.Nodes[i].Position, graph.Nodes[j].Position);
            float distSquared = Vector3LengthSqr(diff);
            
            if (distSquared > 0.0001f && distSquared < 1000.0f)
            {
                float dist = sqrtf(distSquared);
                float repulse = k_squared / dist;
                Vector3 force = Vector3Scale(Vector3Normalize(diff), repulse);
                
                graph.Nodes[i].Force = Vector3Add(graph.Nodes[i].Force, force);
                graph.Nodes[j].Force = Vector3Subtract(graph.Nodes[j].Force, force);
            }
        }
    }

    // Attraction (Hooke's Law on edges)
    for (const auto &edge : graph.Edges)
    {
        Vector3 diff = Vector3Subtract(graph.Nodes[edge.SourceIndex].Position, graph.Nodes[edge.TargetIndex].Position);
        float dist = Vector3Length(diff);
        
        if (dist > 0.0001f)
        {
            float attract = (dist * dist) / k;
            Vector3 force = Vector3Scale(Vector3Normalize(diff), attract);
            
            graph.Nodes[edge.SourceIndex].Force = Vector3Subtract(graph.Nodes[edge.SourceIndex].Force, force);
            graph.Nodes[edge.TargetIndex].Force = Vector3Add(graph.Nodes[edge.TargetIndex].Force, force);
        }
    }

    // Gravity to center to prevent drift
    for (auto &node : graph.Nodes)
    {
        Vector3 centerDiff = Vector3Subtract({0.0f, 0.0f, 0.0f}, node.Position);
        float dist = Vector3Length(centerDiff);
        if (dist > 0.0001f)
        {
            Vector3 gravity = Vector3Scale(Vector3Normalize(centerDiff), 0.05f * dist);
            node.Force = Vector3Add(node.Force, gravity);
        }
    }

    // Integration step
    float maxVelocity = 10.0f;
    for (auto &node : graph.Nodes)
    {
        node.Velocity = Vector3Add(node.Velocity, Vector3Scale(node.Force, deltaTime));
        node.Velocity = Vector3Scale(node.Velocity, 0.9f); // Dampening
        
        float velLength = Vector3Length(node.Velocity);
        if (velLength > maxVelocity)
        {
            node.Velocity = Vector3Scale(Vector3Normalize(node.Velocity), maxVelocity);
        }
        
        node.Position = Vector3Add(node.Position, Vector3Scale(node.Velocity, deltaTime));
    }
}
