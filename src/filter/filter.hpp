#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

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

// Checks if a filter is valid based on the provided argument.
bool isFilterValid(const Filter &filter);
// Sorts the provided list of filters based on the order in the enum definition.
void sortFilters(std::vector<Filter> &filters);
// Parses a size format string into a SizeStruct. Mainly used internally,
// exposed for testing.
SizeStruct parseSize(std::string_view s);
// Returns a boolean indicating whether the file satisfies the filter or not.
bool applyFilter(const fs::path &file, const Filter &filter);
