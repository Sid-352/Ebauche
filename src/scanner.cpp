#include "scanner.h"
#include <filesystem>
#include <unordered_map>
#include <raylib.h>

namespace fs = std::filesystem;

void ScanDirectory(const std::string &rootPath, Graph &outGraph)
{
    std::unordered_map<std::string, size_t> pathToIndex;
    fs::path root(rootPath);
    
    std::string canonicalRoot = root.lexically_normal().string();
    if (!canonicalRoot.empty() && (canonicalRoot.back() == '\\' || canonicalRoot.back() == '/')) {
        canonicalRoot.pop_back();
    }

    Node rootNode;
    rootNode.Position = {0.0f, 0.0f, 0.0f};
    rootNode.Velocity = {0.0f, 0.0f, 0.0f};
    rootNode.Force = {0.0f, 0.0f, 0.0f};
    rootNode.IsDirectory = true;
    rootNode.Radius = 1.0f;
    std::string rootNameStr = root.filename().string();
    if (rootNameStr.empty()) rootNameStr = canonicalRoot;
    strncpy_s(rootNode.Name, sizeof(rootNode.Name), rootNameStr.c_str(), _TRUNCATE);

    outGraph.Nodes.push_back(rootNode);
    pathToIndex[canonicalRoot] = 0;

    std::error_code ec;
    auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec);
    auto end = fs::recursive_directory_iterator();

    while (it != end && !ec)
    {
        const auto &entry = *it;
        std::string parentPath = entry.path().parent_path().lexically_normal().string();
        if (!parentPath.empty() && (parentPath.back() == '\\' || parentPath.back() == '/')) {
            parentPath.pop_back();
        }
        std::string currentPath = entry.path().lexically_normal().string();
        if (!currentPath.empty() && (currentPath.back() == '\\' || currentPath.back() == '/')) {
            currentPath.pop_back();
        }

        if (pathToIndex.find(parentPath) != pathToIndex.end())
        {
            size_t parentIndex = pathToIndex[parentPath];
            size_t currentIndex = outGraph.Nodes.size();

            Node childNode;
            childNode.Position = {
                outGraph.Nodes[parentIndex].Position.x + ((float)GetRandomValue(-100, 100) / 1000.0f),
                outGraph.Nodes[parentIndex].Position.y + ((float)GetRandomValue(-100, 100) / 1000.0f),
                outGraph.Nodes[parentIndex].Position.z + ((float)GetRandomValue(-100, 100) / 1000.0f)
            };
            childNode.Velocity = {0.0f, 0.0f, 0.0f};
            childNode.Force = {0.0f, 0.0f, 0.0f};
            
            bool isDir = false;
            try { isDir = entry.is_directory(ec); } catch (...) {}
            
            childNode.IsDirectory = isDir;
            childNode.Radius = childNode.IsDirectory ? 0.75f : 0.25f;
            std::string childNameStr = entry.path().filename().string();
            strncpy_s(childNode.Name, sizeof(childNode.Name), childNameStr.c_str(), _TRUNCATE);

            outGraph.Nodes.push_back(childNode);
            pathToIndex[currentPath] = currentIndex;

            Edge edge;
            edge.SourceIndex = parentIndex;
            edge.TargetIndex = currentIndex;
            outGraph.Edges.push_back(edge);
        }

        it.increment(ec);
    }
}
