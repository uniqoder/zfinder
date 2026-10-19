#pragma once

#include "filter/filter.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct Args {
  std::string base_dir;
  std::vector<Filter> filters;
  bool do_threading = true;
  bool show_colors = true;
  uint32_t max_depth = 9999;
};

Args parse_arguments(int argc, char *argv[]);
