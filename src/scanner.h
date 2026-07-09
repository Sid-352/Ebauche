#pragma once

#include "graph.h"
#include <atomic>
#include <string>

void ScanDirectory(const std::string &rootPath, Graph &outGraph, std::atomic<size_t> *progressCounter = nullptr,
                   std::atomic<bool> *cancelFlag = nullptr);
