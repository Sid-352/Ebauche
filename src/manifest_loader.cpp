#include "manifest_loader.h"
#include <fstream>

bool LoadGraphManifest(const std::string &path, Graph &outGraph)
{
    std::ifstream file(path + ".bin", std::ios::binary);
    if (!file.is_open())
        return false;

    size_t nodeCount = 0;
    file.read(reinterpret_cast<char *>(&nodeCount), sizeof(nodeCount));
    if (file.fail())
        return false;
    outGraph.Nodes.resize(nodeCount);
    file.read(reinterpret_cast<char *>(outGraph.Nodes.data()), nodeCount * sizeof(Node));
    if (file.fail())
        return false;

    size_t edgeCount = 0;
    file.read(reinterpret_cast<char *>(&edgeCount), sizeof(edgeCount));
    if (file.fail())
        return false;
    outGraph.Edges.resize(edgeCount);
    file.read(reinterpret_cast<char *>(outGraph.Edges.data()), edgeCount * sizeof(Edge));
    if (file.fail())
        return false;

    return true;
}

bool SaveGraphManifest(const std::string &path, const Graph &graph)
{
    std::ofstream file(path + ".bin", std::ios::binary | std::ios::trunc);
    if (!file.is_open())
        return false;

    size_t nodeCount = graph.Nodes.size();
    file.write(reinterpret_cast<const char *>(&nodeCount), sizeof(nodeCount));
    file.write(reinterpret_cast<const char *>(graph.Nodes.data()), nodeCount * sizeof(Node));

    size_t edgeCount = graph.Edges.size();
    file.write(reinterpret_cast<const char *>(&edgeCount), sizeof(edgeCount));
    file.write(reinterpret_cast<const char *>(graph.Edges.data()), edgeCount * sizeof(Edge));

    file.flush();
    return !file.fail();
}
