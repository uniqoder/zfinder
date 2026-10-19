#pragma once

#include <string>
#include <string_view>
#include <vector>

// The order of filter names matters for deciding which filter to use first
enum FilterType { Type, Size, Extension, Name };
struct Filter {
  FilterType type;
  std::string arg;
};

enum SizeType { Equal, Less, LessEqual, More, MoreEqual, Range, Unknown };
struct SizeStruct {
  SizeType type;
  int64_t a; // used in most cases when only one value is required.
  int64_t b; // used when two values are required, like in ranges.
};

bool isFilterValid(const Filter &filter);
void sortFilters(std::vector<Filter> &filters);
SizeStruct parseSize(std::string_view s);
void applyFilter(std::vector<std::string> &candidates, const Filter &filter);
