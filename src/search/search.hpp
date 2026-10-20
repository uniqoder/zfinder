#pragma once

#include "filter/filter.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

struct SearchResult {
  fs::path item_path;
  std::vector<size_t> match_starts;
  std::vector<size_t> match_ends;
  size_t rank;
};

void sortSearchResults(std::vector<SearchResult> &results);
std::vector<SearchResult> performSearch(const fs::path &root,
                                        const std::vector<Filter> &filters,
                                        uint32_t max_depth);
