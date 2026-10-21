#include "cli/argparser.hpp"
#include "filter/filter.hpp"
#include "search/search.hpp"
#include <filesystem>
#include <iostream>

std::string getFilterName(const Filter &filter) {
  switch (filter.type) {
  case FilterType::Name:
    return "Name";
  case FilterType::Type:
    return "Type";
  case FilterType::Size:
    return "Size";
  case FilterType::Extension:
    return "Extension";
  default:
    return "<Unknown>";
  }
}

int main(int argc, char *argv[]) {
  Args args = parse_arguments(argc, argv);

  if (!std::filesystem::exists(args.base_dir)) {
    std::cerr << "Error: " << "Either the base directory \"" << args.base_dir
              << "\" doesn't exist, or we can't access it.\n";
    return 1;
  }

  if (args.filters.size() == 0) {
    std::cerr << "Error: No filters provided. Use --help flag to see usage.\n";
    return 2;
  }

  for (const auto &filter : args.filters) {
    if (!isFilterValid(filter)) {
      std::cerr << "Error: Invalid argument '" << filter.arg << "' to "
                << getFilterName(filter)
                << " filter. Use --help flag to see the usage.\n";
      return 3;
    }
  }

  sortFilters(args.filters);
  auto results = performSearch(args.base_dir, args.filters, args.max_depth);
  sortSearchResults(results);

  for (const auto &result : results) {
    std::cout << result.item_path.string() << "\n";
  }

  return 0;
}
