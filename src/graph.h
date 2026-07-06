#pragma once

#include <raylib.h>
#include <string>
#include <vector>

struct Node
{
    char Name[256];
    bool IsDirectory;
    float Mass;
    float Radius;
    int Depth;
    Vector3 Position;
    Vector3 Velocity = {0.0f, 0.0f, 0.0f};

    size_t ParentIndex = (size_t)-1;
    float OrbitRadius = 0.0f;
    float OrbitAngle = 0.0f;
    float OrbitSpeed = 0.0f;
    float YOffset = 0.0f;
    float OrbitTilt = 0.0f;
    float SpinAngle = 0.0f;
    float SpinSpeed = 0.0f;
};

struct Edge
{
    size_t SourceIndex;
    size_t TargetIndex;
};

struct Graph
{
    std::vector<Node> Nodes;
    std::vector<Edge> Edges;
};
