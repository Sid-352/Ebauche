#include "scanner.h"
#include "logger.h"
#include <filesystem>
#include <raylib.h>
#include <unordered_map>
#include <cstdio>

namespace fs = std::filesystem;

void ScanDirectory(const std::string &rootPath, Graph &outGraph, std::atomic<size_t> *progressCounter)
{
    std::unordered_map<std::string, size_t> pathToIndex;
    fs::path root(rootPath);

    std::string canonicalRoot = reinterpret_cast<const char *>(root.lexically_normal().u8string().c_str());
    if (!canonicalRoot.empty() && (canonicalRoot.back() == '\\' || canonicalRoot.back() == '/'))
    {
        canonicalRoot.pop_back();
    }

    Node rootNode;
    rootNode.Position = {0.0f, 0.0f, 0.0f};
    rootNode.IsDirectory = true;
    rootNode.Depth = 0;
    std::string rootNameStr = reinterpret_cast<const char *>(root.filename().u8string().c_str());
    if (rootNameStr.empty())
        rootNameStr = canonicalRoot;
    snprintf(rootNode.Name, sizeof(rootNode.Name), "%s", rootNameStr.c_str());
    snprintf(rootNode.Path, sizeof(rootNode.Path), "%s", canonicalRoot.c_str());

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
            std::string parentPath =
                reinterpret_cast<const char *>(entry.path().parent_path().lexically_normal().u8string().c_str());
            if (!parentPath.empty() && (parentPath.back() == '\\' || parentPath.back() == '/'))
            {
                parentPath.pop_back();
            }
            std::string currentPath =
                reinterpret_cast<const char *>(entry.path().lexically_normal().u8string().c_str());
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
                uint64_t mass = 1;
                try
                {
                    isDir = entry.is_directory(ec);
                    if (!isDir && !ec)
                    {
                        std::error_code sizeEc;
                        uintmax_t size = fs::file_size(entry, sizeEc);
                        if (!sizeEc)
                            mass = static_cast<uint64_t>(size);
                    }
                }
                catch (...)
                {
                }

                childNode.IsDirectory = isDir;
                childNode.Mass = mass;
                std::string childNameStr = reinterpret_cast<const char *>(entry.path().filename().u8string().c_str());
                snprintf(childNode.Name, sizeof(childNode.Name), "%s", childNameStr.c_str());
                snprintf(childNode.Path, sizeof(childNode.Path), "%s", currentPath.c_str());

                outGraph.Nodes.push_back(childNode);
                pathToIndex[currentPath] = currentIndex;

                Edge edge;
                edge.SourceIndex = parentIndex;
                edge.TargetIndex = currentIndex;
                outGraph.Edges.push_back(edge);
            }

            if (progressCounter)
            {
                (*progressCounter)++;
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

    uint64_t rngState = 123456789ULL;
    auto pcg32 = [&rngState]() -> uint32_t
    {
        const uint64_t oldState = rngState;
        rngState = (oldState * 6364136223846793005ULL) + 1;
        const auto xorshifted = static_cast<uint32_t>(((oldState >> 18u) ^ oldState) >> 27u);
        const auto rot = static_cast<uint32_t>(oldState >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    };
    auto nextFloat = [&pcg32]() -> float
    { return static_cast<float>(pcg32() & 0xFFFFFF) / static_cast<float>(0xFFFFFF); };

    for (size_t i = 0; i < outGraph.Nodes.size(); i++)
    {
        auto &children = adj[i];
        size_t numChildren = children.size();
        if (numChildren == 0)
        {
            continue;
        }

        std::sort(children.begin(), children.end(),
                  [&outGraph](size_t a, size_t b) { return outGraph.Nodes[a].Mass > outGraph.Nodes[b].Mass; });

        for (size_t childIndex = 0; childIndex < numChildren; childIndex++)
        {
            size_t childIdx = children[childIndex];

            const float theta = nextFloat() * 6.28318f;
            const float rRandom = nextFloat();

            struct GalaxyParams
            {
                float rBase, rMult;
                float thickness;
                float tiltLimit;
                float keplerBase;
                float orbitSpeedMin, orbitSpeedMax;
                float spinSpeedMin, spinSpeedMax;
                float eccMin, eccMax;
                float jitterSpeedMin, jitterSpeedMax;
                float jitterAmpMin, jitterAmpMax;
            };

            GalaxyParams params;
            if (outGraph.Nodes[childIdx].IsDirectory)
            {
                params = {10.0f, 3500.0f, 300.0f, 0.5f, 10.0f, 0.05f, 0.15f, -0.05f,
                          0.05f, 0.3f,    0.8f,   0.9f, 1.1f,  0.05f, 0.15f};
            }
            else
            {
                params = {1.0f, 150.0f, 20.0f, 0.15f, 30.0f, 0.5f,  1.5f, -0.2f,
                          0.2f, 0.5f,   0.8f,  0.8f,  1.2f,  0.02f, 0.10f};
            }

            auto randFloat = [&nextFloat](float min, float max) { return min + ((max - min) * nextFloat()); };

            float radius = params.rBase + ((rRandom * rRandom) * params.rMult);
            const float thick = params.thickness * (1.0f - rRandom);
            float yOffset = randFloat(-1.0f, 1.0f) * thick;
            float tilt = randFloat(-params.tiltLimit, params.tiltLimit);

            outGraph.Nodes[childIdx].ParentIndex = i;
            outGraph.Nodes[childIdx].OrbitRadius = radius;
            outGraph.Nodes[childIdx].OrbitAngle = theta;

            const float keplerSpeed = params.keplerBase / sqrtf(radius + 1.0f);
            outGraph.Nodes[childIdx].OrbitSpeed = keplerSpeed * randFloat(params.orbitSpeedMin, params.orbitSpeedMax);

            outGraph.Nodes[childIdx].YOffset = yOffset;
            outGraph.Nodes[childIdx].OrbitTilt = tilt;
            outGraph.Nodes[childIdx].SpinAngle = randFloat(0.0f, 6.28f);
            outGraph.Nodes[childIdx].SpinSpeed = randFloat(params.spinSpeedMin, params.spinSpeedMax);

            outGraph.Nodes[childIdx].OrbitRotation = randFloat(0.0f, 6.28f);
            outGraph.Nodes[childIdx].Eccentricity = randFloat(params.eccMin, params.eccMax);
            outGraph.Nodes[childIdx].RadiusJitterPhase = randFloat(0.0f, 6.28f);
            outGraph.Nodes[childIdx].RadiusJitterSpeed =
                outGraph.Nodes[childIdx].OrbitSpeed * randFloat(params.jitterSpeedMin, params.jitterSpeedMax);
            outGraph.Nodes[childIdx].RadiusJitterAmp = radius * randFloat(params.jitterAmpMin, params.jitterAmpMax);

            outGraph.Nodes[childIdx].Position = {
                outGraph.Nodes[i].Position.x + (cosf(theta) * radius), outGraph.Nodes[i].Position.y + yOffset,
                outGraph.Nodes[i].Position.z + (sinf(theta) * radius * (1.0f - outGraph.Nodes[childIdx].Eccentricity))};
        }
    }

    LOG_INFO("Galaxy Layout complete. Cleaning up local variables...");
}
