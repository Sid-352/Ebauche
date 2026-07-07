#pragma once

#include "graph.h"
#include <string>

bool LoadGraphManifest(const std::string &path, Graph &outGraph);
bool SaveGraphManifest(const std::string &path, const Graph &graph);
