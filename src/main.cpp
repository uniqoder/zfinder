#include "cli/argparser.hpp"
#include "filter/filter.hpp"
#include "search/search.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <functional>
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
    std::cerr << "Error: Either the base directory \"" << args.base_dir
              << "\" doesn't exist, or we can't access it.\n";
    return 1;
  }

  if (!std::filesystem::is_directory(args.base_dir)) {
    std::cerr << "Error: The provided path is not a directory: "
              << args.base_dir << "\n";
    return 2;
  }

  if (args.filters.size() == 0) {
    std::cerr << "Error: No filters provided. Use --help flag to see usage.\n";
    return 3;
  }

  for (const auto &filter : args.filters) {
    if (!isFilterValid(filter)) {
      std::cerr << "Error: Invalid argument '" << filter.arg << "' to "
                << getFilterName(filter)
                << " filter. Use --help flag to see the usage.\n";
      return 4;
    }
  }

  auto contains_file_type =
      std::find_if(args.filters.begin(), args.filters.end(), [](Filter f) {
        return f.type == FilterType::Type && f.arg == "f";
      }) != args.filters.end();

  if (!args.dir_size && !contains_file_type) {
    args.filters.push_back({FilterType::Type, "f"});
  }

  sortFilters(args.filters);

  const auto printResult = [](const SearchResult &result) {
    std::cout << std::round(result.accuracy * 100) << "% "
              << result.item_path.string() << "\n";
  };
  auto results =
      performSearch(args.base_dir, args.filters, args.max_depth, printResult);
  sortSearchResults(results);

  std::cout << "\nBest matches:\n";
  for (const auto &result : results) {
    if (result.accuracy < 0.7f)
      continue;
    int score = std::round(result.accuracy * 100);
    std::cout << score << "% " << result.item_path.string() << "\n";
  }

  return 0;
}
