#include "scanner.h"
#include "logger.h"
#include <filesystem>
#include <raylib.h>
#include <unordered_map>

namespace fs = std::filesystem;

void ScanDirectory(const std::string &rootPath, Graph &outGraph)
{
    std::unordered_map<std::string, size_t> pathToIndex;
    fs::path root(rootPath);

    std::string canonicalRoot = root.lexically_normal().string();
    if (!canonicalRoot.empty() && (canonicalRoot.back() == '\\' || canonicalRoot.back() == '/'))
    {
        canonicalRoot.pop_back();
    }

    Node rootNode;
    rootNode.Position = {0.0f, 0.0f, 0.0f};
    rootNode.IsDirectory = true;
    rootNode.Depth = 0;
    rootNode.Radius = 1.0f;
    std::string rootNameStr = root.filename().string();
    if (rootNameStr.empty())
        rootNameStr = canonicalRoot;
    strncpy_s(rootNode.Name, sizeof(rootNode.Name), rootNameStr.c_str(), _TRUNCATE);

    outGraph.Nodes.push_back(rootNode);
    pathToIndex[canonicalRoot] = 0;

    std::error_code ec;
    auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec);
    auto end = fs::recursive_directory_iterator();

    while (it != end && !ec)
    {
        try
        {
            const auto &entry = *it;
            std::string parentPath = entry.path().parent_path().lexically_normal().string();
            if (!parentPath.empty() && (parentPath.back() == '\\' || parentPath.back() == '/'))
            {
                parentPath.pop_back();
            }
            std::string currentPath = entry.path().lexically_normal().string();
            if (!currentPath.empty() && (currentPath.back() == '\\' || currentPath.back() == '/'))
            {
                currentPath.pop_back();
            }

            if (pathToIndex.find(parentPath) != pathToIndex.end())
            {
                size_t parentIndex = pathToIndex[parentPath];
                size_t currentIndex = outGraph.Nodes.size();

                Node childNode;
                childNode.Position = {0.0f, 0.0f, 0.0f}; // Set during layout pass
                childNode.Depth = outGraph.Nodes[parentIndex].Depth + 1;
                bool isDir = false;
                float mass = 1.0f;
                try
                {
                    isDir = entry.is_directory(ec);
                    if (!isDir && !ec)
                    {
                        std::error_code sizeEc;
                        uintmax_t size = fs::file_size(entry, sizeEc);
                        if (!sizeEc)
                            mass = (float)size;
                    }
                }
                catch (...)
                {
                }

                childNode.IsDirectory = isDir;
                childNode.Mass = mass;
                std::string childNameStr = entry.path().filename().string();
                strncpy_s(childNode.Name, sizeof(childNode.Name), childNameStr.c_str(), _TRUNCATE);

                outGraph.Nodes.push_back(childNode);
                pathToIndex[currentPath] = currentIndex;

                Edge edge;
                edge.SourceIndex = parentIndex;
                edge.TargetIndex = currentIndex;
                outGraph.Edges.push_back(edge);
            }

            if (outGraph.Nodes.size() % 10000 == 0)
            {
                LOG_INFO("Scanned %zu nodes...", outGraph.Nodes.size());
            }

            it.increment(ec);
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Standard exception encountered, skipping file: %s", e.what());
            it.increment(ec);
        }
        catch (...)
        {
            LOG_ERROR("Unknown exception encountered, skipping file...");
            it.increment(ec);
        }
    }

    LOG_INFO("ScanDirectory loop finished. Executing bottom-up Mass accumulation...");

    for (size_t i = outGraph.Edges.size(); i-- > 0;)
    {
        size_t parentIdx = outGraph.Edges[i].SourceIndex;
        size_t childIdx = outGraph.Edges[i].TargetIndex;
        outGraph.Nodes[parentIdx].Mass += outGraph.Nodes[childIdx].Mass;
    }

    for (auto &node : outGraph.Nodes)
    {
        node.Radius = node.IsDirectory ? (0.5f + log10f(node.Mass + 1.0f) * 2.0f) : 0.25f;
    }

    LOG_INFO("Executing top-down Procedural Galaxy Layout...");

    std::vector<std::vector<size_t>> adj(outGraph.Nodes.size());
    for (const auto &edge : outGraph.Edges)
    {
        adj[edge.SourceIndex].push_back(edge.TargetIndex);
    }

    if (!outGraph.Nodes.empty())
    {
        outGraph.Nodes[0].Position = {0.0f, 0.0f, 0.0f};
    }

    for (size_t i = 0; i < outGraph.Nodes.size(); i++)
    {
        auto &children = adj[i];
        size_t numChildren = children.size();
        if (numChildren == 0)
            continue;

        std::sort(children.begin(), children.end(),
                  [&outGraph](size_t a, size_t b) { return outGraph.Nodes[a].Mass > outGraph.Nodes[b].Mass; });

        for (size_t c = 0; c < numChildren; c++)
        {
            size_t childIdx = children[c];

            float theta = static_cast<float>(GetRandomValue(0, 628318)) / 100000.0f;
            float rRandom = static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f;

            if (outGraph.Nodes[childIdx].IsDirectory)
            {
                float r = outGraph.Nodes[i].Radius + 10.0f + ((rRandom * rRandom) * 3500.0f);
                const float thickness = 300.0f * (1.0f - rRandom);
                float yOffset = (static_cast<float>(GetRandomValue(-1000, 1000)) / 1000.0f) * thickness;
                float tilt = (static_cast<float>(GetRandomValue(-500, 500)) / 1000.0f);

                outGraph.Nodes[childIdx].ParentIndex = i;
                outGraph.Nodes[childIdx].OrbitRadius = r;
                outGraph.Nodes[childIdx].OrbitAngle = theta;

                float keplerSpeed = 10.0f / sqrtf(r + 1.0f);
                outGraph.Nodes[childIdx].OrbitSpeed =
                    keplerSpeed * (static_cast<float>(GetRandomValue(50, 150)) / 1000.0f);

                outGraph.Nodes[childIdx].YOffset = yOffset;
                outGraph.Nodes[childIdx].OrbitTilt = tilt;
                outGraph.Nodes[childIdx].SpinAngle = (static_cast<float>(GetRandomValue(0, 628)) / 100.0f);
                outGraph.Nodes[childIdx].SpinSpeed = (static_cast<float>(GetRandomValue(-50, 50)) / 1000.0f);

                outGraph.Nodes[childIdx].OrbitRotation = static_cast<float>(GetRandomValue(0, 628)) / 100.0f;
                outGraph.Nodes[childIdx].Eccentricity = static_cast<float>(GetRandomValue(30, 80)) / 100.0f;
                outGraph.Nodes[childIdx].RadiusJitterPhase = static_cast<float>(GetRandomValue(0, 628)) / 100.0f;
                outGraph.Nodes[childIdx].RadiusJitterSpeed =
                    outGraph.Nodes[childIdx].OrbitSpeed * (static_cast<float>(GetRandomValue(90, 110)) / 100.0f);
                outGraph.Nodes[childIdx].RadiusJitterAmp = r * (static_cast<float>(GetRandomValue(5, 15)) / 100.0f);

                outGraph.Nodes[childIdx].Position = {
                    outGraph.Nodes[i].Position.x + cosf(theta) * r, outGraph.Nodes[i].Position.y + yOffset,
                    outGraph.Nodes[i].Position.z + sinf(theta) * r * (1.0f - outGraph.Nodes[childIdx].Eccentricity)};
            }
            else
            {
                float r = outGraph.Nodes[i].Radius + 1.0f + ((rRandom * rRandom) * 150.0f);
                const float thickness = 20.0f * (1.0f - rRandom);
                float yOffset = (static_cast<float>(GetRandomValue(-1000, 1000)) / 1000.0f) * thickness;
                float tilt = (static_cast<float>(GetRandomValue(-150, 150)) / 1000.0f);

                outGraph.Nodes[childIdx].ParentIndex = i;
                outGraph.Nodes[childIdx].OrbitRadius = r;
                outGraph.Nodes[childIdx].OrbitAngle = theta;

                float keplerSpeed = 30.0f / sqrtf(r + 1.0f);
                outGraph.Nodes[childIdx].OrbitSpeed =
                    keplerSpeed * (static_cast<float>(GetRandomValue(50, 150)) / 100.0f);

                outGraph.Nodes[childIdx].YOffset = yOffset;
                outGraph.Nodes[childIdx].OrbitTilt = tilt;
                outGraph.Nodes[childIdx].SpinAngle = (static_cast<float>(GetRandomValue(0, 628)) / 100.0f);
                outGraph.Nodes[childIdx].SpinSpeed = (static_cast<float>(GetRandomValue(-200, 200)) / 1000.0f);

                outGraph.Nodes[childIdx].OrbitRotation = static_cast<float>(GetRandomValue(0, 628)) / 100.0f;
                outGraph.Nodes[childIdx].Eccentricity = static_cast<float>(GetRandomValue(50, 80)) / 100.0f;
                outGraph.Nodes[childIdx].RadiusJitterPhase = static_cast<float>(GetRandomValue(0, 628)) / 100.0f;
                outGraph.Nodes[childIdx].RadiusJitterSpeed =
                    outGraph.Nodes[childIdx].OrbitSpeed * (static_cast<float>(GetRandomValue(80, 120)) / 100.0f);
                outGraph.Nodes[childIdx].RadiusJitterAmp = r * (static_cast<float>(GetRandomValue(2, 10)) / 100.0f);

                outGraph.Nodes[childIdx].Position = {
                    outGraph.Nodes[i].Position.x + cosf(theta) * r, outGraph.Nodes[i].Position.y + yOffset,
                    outGraph.Nodes[i].Position.z + sinf(theta) * r * (1.0f - outGraph.Nodes[childIdx].Eccentricity)};
            }
        }
    }

    LOG_INFO("Galaxy Layout complete. Cleaning up local variables...");
}
