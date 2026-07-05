#pragma once

#include "graph.h"
#include <string>

bool LoadGraphManifest(const std::string &path, Graph &outGraph);
void SaveGraphManifest(const std::string &path, const Graph &graph);
