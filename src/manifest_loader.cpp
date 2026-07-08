#include "manifest_loader.h"
#include <cstdint>
#include <fstream>

static const uint32_t MANIFEST_MAGIC = 0x45424155; // 'EBAU'
static const uint16_t MANIFEST_VERSION = 1;

bool LoadGraphManifest(const std::string &path, Graph &outGraph)
{
    std::ifstream file(path + ".bin", std::ios::binary);
    if (!file.is_open())
        return false;

    uint32_t magic = 0;
    file.read(reinterpret_cast<char *>(&magic), sizeof(magic));
    if (file.fail() || magic != MANIFEST_MAGIC)
        return false;

    uint16_t version = 0;
    file.read(reinterpret_cast<char *>(&version), sizeof(version));
    if (file.fail() || version != MANIFEST_VERSION)
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

    file.write(reinterpret_cast<const char *>(&MANIFEST_MAGIC), sizeof(MANIFEST_MAGIC));
    if (file.fail())
        return false;

    file.write(reinterpret_cast<const char *>(&MANIFEST_VERSION), sizeof(MANIFEST_VERSION));
    if (file.fail())
        return false;

    size_t nodeCount = graph.Nodes.size();
    file.write(reinterpret_cast<const char *>(&nodeCount), sizeof(nodeCount));
    if (file.fail())
        return false;

    file.write(reinterpret_cast<const char *>(graph.Nodes.data()), nodeCount * sizeof(Node));
    if (file.fail())
        return false;

    size_t edgeCount = graph.Edges.size();
    file.write(reinterpret_cast<const char *>(&edgeCount), sizeof(edgeCount));
    if (file.fail())
        return false;

    file.write(reinterpret_cast<const char *>(graph.Edges.data()), edgeCount * sizeof(Edge));
    if (file.fail())
        return false;

    file.flush();
    return !file.fail();
}
