#pragma once

#include <string>
#include <vector>

// The order of filter names matters for deciding which filter to use first
enum FilterType { Type, Size, Extension, Name };

struct Filter {
  FilterType type;
  std::string arg;
};

bool isFilterValid(const Filter &filter);
void sortFilters(std::vector<Filter> &filters);
void applyFilter(std::vector<std::string> &candidates, const Filter &filter);
