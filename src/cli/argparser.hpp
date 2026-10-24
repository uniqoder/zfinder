#pragma once

#include "filter/filter.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct Args {
  std::string base_dir;
  std::vector<Filter> filters;
  uint32_t max_depth = 9999;
  bool dir_size = false;
  bool show_best = false;
};

Args parse_arguments(int argc, char *argv[]);
