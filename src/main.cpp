#include "cli/argparser.hpp"
#include <iostream>

std::string getFilterName(FilterType type) {
  switch (type) {
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

  std::cout << "Base directory: " << args.base_dir << "\n"
            << "Recursion depth: " << args.max_depth << "\n"
            << "Multi-threading: "
            << (args.do_threading ? "enabled" : "disabled") << "\n"
            << "Colored output: " << (args.show_colors ? "enabled" : "disabled")
            << "\n"
            << "Filters:";

  for (const auto &filter : args.filters) {
    std::cout << "\n  " << getFilterName(filter.type) << " " << filter.arg;
  }

  std::cout << std::endl;

  return 0;
}
