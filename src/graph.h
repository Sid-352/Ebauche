#pragma once

#include <raylib.h>
#include <string>
#include <vector>

struct Node
{
    Vector3 Position;
    Vector3 Velocity;
    Vector3 Force;
    bool IsDirectory;
    float Radius;
    char Name[256];
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
