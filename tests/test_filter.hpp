#pragma once

#include "filter/filter.hpp"
#include <cassert>
#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace filter {

void check(bool condition, const std::string &msg) {
  if (!condition)
    throw std::runtime_error(msg);
}

void test_name_filter_validity() {
  Filter valid_name1 = {.type = FilterType::Name, .arg = "test"};
  Filter valid_name2 = {.type = FilterType::Name, .arg = "-!./hello"};

  Filter invalid_name1 = {.type = FilterType::Name, .arg = ""};

  check(isFilterValid(valid_name1) && isFilterValid(valid_name2),
        "name filter should be valid when a name is provided");
  check(!isFilterValid(invalid_name1),
        "name filter should be invalid when no name is provided");
}

void test_extension_filter_validity() {
  Filter valid_ext1 = {.type = FilterType::Extension, .arg = "txt"};
  Filter valid_ext2 = {.type = FilterType::Extension, .arg = "tar.gz"};

  Filter invalid_ext1 = {.type = FilterType::Extension, .arg = ""};

  check(isFilterValid(valid_ext1) && isFilterValid(valid_ext2),
        "extension filter should be valid when an extension is provided");
  check(!isFilterValid(invalid_ext1),
        "extension filter should be invalid when no extension is provided");
}

void test_type_filter_validity() {
  Filter valid_type1 = {.type = FilterType::Type, .arg = "f"};
  Filter valid_type2 = {.type = FilterType::Type, .arg = "d"};
  Filter valid_type3 = {.type = FilterType::Type, .arg = "l"};

  Filter invalid_type1 = {.type = FilterType::Type, .arg = "lf"};
  Filter invalid_type2 = {.type = FilterType::Type, .arg = ""};

  check(isFilterValid(valid_type1) && isFilterValid(valid_type2) &&
            isFilterValid(valid_type3),
        "type filter should be valid when either one of 'f', 'd', or 'l' is "
        "provided");
  check(!isFilterValid(invalid_type1) && !isFilterValid(invalid_type2),
        "type filter should be invalid when neither one of 'f', 'd', 'l' is "
        "provided");
}

void test_size_filter_validity() {
  Filter valid_size1 = {.type = FilterType::Size, .arg = "10"};
  Filter valid_size2 = {.type = FilterType::Size, .arg = "1029K"};
  Filter valid_size3 = {.type = FilterType::Size, .arg = "<2g"};
  Filter valid_size4 = {.type = FilterType::Size, .arg = "<=1t"};
  Filter valid_size5 = {.type = FilterType::Size, .arg = ">12"};
  Filter valid_size6 = {.type = FilterType::Size, .arg = ">=50m"};
  Filter valid_size7 = {.type = FilterType::Size, .arg = "23M..1g"};

  Filter invalid_size1 = {.type = FilterType::Size, .arg = ""};
  Filter invalid_size2 = {.type = FilterType::Size, .arg = "=50M"};
  Filter invalid_size3 = {.type = FilterType::Size, .arg = "<24B"};
  Filter invalid_size4 = {.type = FilterType::Size, .arg = "a..b"};
  Filter invalid_size5 = {.type = FilterType::Size, .arg = "!45M..8K"};

  check(isFilterValid(valid_size1) && isFilterValid(valid_size2) &&
            isFilterValid(valid_size3) && isFilterValid(valid_size4) &&
            isFilterValid(valid_size5) && isFilterValid(valid_size6) &&
            isFilterValid(valid_size7),
        "size filter should be valid when the correct format is provided");
  check(!isFilterValid(invalid_size1) && !isFilterValid(invalid_size2) &&
            !isFilterValid(invalid_size3) && !isFilterValid(invalid_size4) &&
            !isFilterValid(invalid_size5),
        "size filter should be invalid when an incorrect format is provided");
}

void test_size_parsing() {
  auto actual1 = parseSize("10");
  auto actual2 = parseSize("19K");
  auto actual3 = parseSize("<2g");
  auto actual4 = parseSize("<=1M");
  auto actual5 = parseSize(">12");
  auto actual6 = parseSize(">=50m");
  auto actual7 = parseSize("23M..1g");

  bool test1 = actual1.type == SizeType::Equal && actual1.a == 10l;
  bool test2 = actual2.type == SizeType::Equal && actual2.a == 19000l;
  bool test3 = actual3.type == SizeType::Less && actual3.a == 2147483648l;
  bool test4 = actual4.type == SizeType::LessEqual && actual4.a == 1000000l;
  bool test5 = actual5.type == SizeType::More && actual5.a == 12l;
  bool test6 = actual6.type == SizeType::MoreEqual && actual6.a == 52428800l;
  bool test7 = actual7.type == SizeType::Range && actual7.a == 23000000l &&
               actual7.b == 1073741824l;

  check(test1 && test2 && test3 && test4 && test5 && test6 && test7,
        "size should be parsed correctly on the right format");
}

void test_sort_filters() {
  std::vector<Filter> filters = {
      {.type = FilterType::Name, .arg = "bin"},
      {.type = FilterType::Type, .arg = "d"},
      {.type = FilterType::Extension, .arg = "cpp"},
      {.type = FilterType::Size, .arg = "<2k"},
  };

  sortFilters(filters);

  check(filters.size() == 4, "sorting filters shouldn't change list length");
  check(filters[0].type == FilterType::Type &&
            filters[1].type == FilterType::Size &&
            filters[2].type == FilterType::Extension &&
            filters[3].type == FilterType::Name,
        "filters should be sorted in the following order: Type, Size, "
        "Extension, Name");
}

std::vector<std::pair<std::string, std::function<void()>>> tests{
    {"name filter validity", test_name_filter_validity},
    {"extension filter validity", test_extension_filter_validity},
    {"type filter validity", test_type_filter_validity},
    {"size filter validity", test_extension_filter_validity},
    {"size parsing", test_size_parsing},
    {"filter sorting", test_sort_filters},
};

size_t test_count() { return tests.size(); }

int test_all() {
  int failures = 0;

  for (const auto &test : tests) {
    try {
      test.second();
      std::cout << "PASS: " << test.first << ".\n";
    } catch (const std::exception &error) {
      failures++;
      std::cerr << "FAIL: " << test.first << ": " << error.what() << ".\n";
    }
  }

  return failures;
}

}; // namespace filter
