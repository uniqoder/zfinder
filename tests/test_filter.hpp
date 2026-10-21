#pragma once

#include "filter/filter.hpp"
#include <cassert>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace filter {

const fs::path ROOT = "./.temp_dir";

void check(bool condition, const std::string &msg) {
  if (!condition)
    throw std::runtime_error(msg);
}

void create_directory_structure() {
  fs::create_directories(ROOT / "depth1");

  std::ofstream(ROOT / "hello.txt") << "Hello World!";
  std::ofstream(ROOT / "backup.tar.gz");
  std::ofstream(ROOT / "depth1/test_name");

  std::ofstream(ROOT / "depth1/sparse.dat");
  fs::resize_file(ROOT / "depth1/sparse.dat", 1024l * 1024l * 1024l); // 1GiB

  fs::create_symlink(ROOT / "hello.txt", ROOT / "depth1/hello - shortcut");
}

void clean_directory_structure() { fs::remove_all(ROOT); }

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

std::string sizeTypeToStr(const SizeType &type) {
  switch (type) {
  case SizeType::Less:
    return "<";
  case SizeType::LessEqual:
    return "<=";
  case SizeType::More:
    return ">";
  case SizeType::MoreEqual:
    return ">=";
  default:
    return "";
  }
}

void test_size_parsing() {
  auto actual1 = parseSize("10");
  auto actual2 = parseSize("19K");
  auto actual3 = parseSize("<2g");
  auto actual4 = parseSize("<=1M");
  auto actual5 = parseSize(">12");
  auto actual6 = parseSize(">=50m");
  auto actual7 = parseSize("23M..1g");

  check(actual1.type == SizeType::Equal && actual1.a == 10l,
        "size parsing mismatch: expected = " + sizeTypeToStr(SizeType::Equal) +
            "10, actual = " + sizeTypeToStr(actual1.type) +
            std::to_string(actual1.a));
  check(actual2.type == SizeType::Equal && actual2.a == 19000l,
        "size parsing mismatch: expected = " + sizeTypeToStr(SizeType::Equal) +
            "19000, actual = " + sizeTypeToStr(actual2.type) +
            std::to_string(actual2.a));
  check(actual3.type == SizeType::Less && actual3.a == 2147483648l,
        "size parsing mismatch: expected = " + sizeTypeToStr(SizeType::Less) +
            "2147483648, actual = " + sizeTypeToStr(actual3.type) +
            std::to_string(actual3.a));
  check(actual4.type == SizeType::LessEqual && actual4.a == 1000000l,
        "size parsing mismatch: expected = " +
            sizeTypeToStr(SizeType::LessEqual) + "1000000, actual = " +
            sizeTypeToStr(actual4.type) + std::to_string(actual4.a));
  check(actual5.type == SizeType::More && actual5.a == 12l,
        "size parsing mismatch: expected = " + sizeTypeToStr(SizeType::More) +
            "12, actual = " + sizeTypeToStr(actual5.type) +
            std::to_string(actual5.a));
  check(actual6.type == SizeType::MoreEqual && actual6.a == 52428800l,
        "size parsing mismatch: expected = " +
            sizeTypeToStr(SizeType::MoreEqual) + "52428800, actual = " +
            sizeTypeToStr(actual6.type) + std::to_string(actual6.a));
  check(actual7.type == SizeType::Range && actual7.a == 23000000l &&
            actual7.b == 1073741824l,
        "size parsing mismatch: expected = 23000000..1073741824, actual = " +
            std::to_string(actual7.a) + ".." + std::to_string(actual7.b));
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

void test_extension_filter() {
  Filter filter1 = {.type = FilterType::Extension, .arg = "txt"};
  Filter filter2 = {.type = FilterType::Extension, .arg = ".tar.gz"};
  Filter filter3 = {.type = FilterType::Extension, .arg = "test"};
  Filter filter4 = {.type = FilterType::Extension, .arg = "cpp"};
  Filter filter5 = {.type = FilterType::Extension, .arg = "full"};

  try {
    check(applyFilter("src/test/hello.txt", filter1).type == Found,
          "extension mismatch for 'src/test/hello.txt': expected = txt");
    check(applyFilter("backup.tar.gz", filter2).type == Found,
          "extension mismatch for 'backup.tar.gz': expected = .tar.gz");
    check(applyFilter("hello.txt.test", filter3).type == Found,
          "extension mismatch for 'hello.txt.test': expected = test");
    check(!applyFilter("src/core/main.c", filter4).type == Found,
          "extension mismatch for 'src/core/main.c': expected = cpp");
    check(applyFilter("bin/full", filter5).type == Found,
          "extension mismatch for 'bin/full': expected = full");
  } catch (const std::exception &) {
    throw;
  }
}

char getItemType(const fs::path &item) {
  if (fs::is_symlink(item))
    return 'l';
  if (fs::is_directory(item))
    return 'd';
  if (fs::is_regular_file(item))
    return 'f';
  return '?';
}

void test_type_filter() {
  if (!fs::exists(ROOT))
    throw std::runtime_error("no directory structure for testing");

  Filter filter_f = {.type = FilterType::Type, .arg = "f"};
  Filter filter_d = {.type = FilterType::Type, .arg = "d"};
  Filter filter_l = {.type = FilterType::Type, .arg = "l"};

  try {
    check(applyFilter(ROOT / "hello.txt", filter_f).type == Found,
          std::string("type mismatch: expected = f, actual = ") +
              getItemType(ROOT / "hello.txt"));
    check(applyFilter(ROOT / "depth1", filter_d).type == Found,
          std::string("type mismatch: expected = d, actual = ") +
              getItemType(ROOT / "depth1"));
    check(applyFilter(ROOT / "depth1/hello - shortcut", filter_l).type == Found,
          std::string("type mismatch: expected = l, actual = ") +
              getItemType(ROOT / "depth1/hello - shortcut"));
    check(applyFilter(ROOT / "hello.txt", filter_d).type == NotFound,
          std::string("type mismatch: expected = d, actual = ") +
              getItemType(ROOT / "hello.txt"));
    check(applyFilter(ROOT / "depth1", filter_f).type == NotFound,
          std::string("type mismatch: expected = f, actual = ") +
              getItemType(ROOT / "depth1"));
    check(applyFilter(ROOT / "hello.txt", filter_l).type == NotFound,
          std::string("type mismatch: expected = l, actual = ") +
              getItemType(ROOT / "hello.txt"));
  } catch (const std::exception &) {
    throw;
  }
}

void test_size_filter() {
  if (!fs::exists(ROOT))
    throw std::runtime_error("no directory structure for testing");

  Filter filter1 = {.type = FilterType::Size, .arg = "12"};
  Filter filter2 = {.type = FilterType::Size, .arg = "<1M"};
  Filter filter3 = {.type = FilterType::Size, .arg = ">=1g"};
  Filter filter4 = {.type = FilterType::Size, .arg = "5..12"};
  Filter filter5 = {.type = FilterType::Size, .arg = "5..13"};

  try {
    check(applyFilter(ROOT / "hello.txt", filter1).type == Found,
          "size mismatch: expected = 12, actual = " +
              std::to_string(fs::file_size(ROOT / "hello.txt")));
    check(applyFilter(ROOT / "backup.tar.gz", filter2).type == Found,
          "size mismatch: expected = <1,000,000, actual = " +
              std::to_string(fs::file_size(ROOT / "backup.tar.gz")));
    check(applyFilter(ROOT / "depth1/sparse.dat", filter3).type == Found,
          "size mismatch: expected = >=1,073,741,824, actual = " +
              std::to_string(fs::file_size(ROOT / "depth1/sparse.dat")));
    check(applyFilter(ROOT / "hello.txt", filter4).type == NotFound,
          "size mismatch: expected = 5..12, actual = " +
              std::to_string(fs::file_size(ROOT / "hello.txt")));
    check(applyFilter(ROOT / "hello.txt", filter5).type == Found,
          "size mismatch: expected = 5..13, actual = " +
              std::to_string(fs::file_size(ROOT / "hello.txt")));
  } catch (const std::exception &) {
    throw;
  }
}

void test_name_filter() {
  Filter filter1 = {.type = FilterType::Name, .arg = "test"};
  Filter filter2 = {.type = FilterType::Name, .arg = "retry.me"};
  Filter filter3 = {.type = FilterType::Name, .arg = "src/main.cpp"};
  Filter filter4 = {.type = FilterType::Name, .arg = "bak.tar"};

  check(applyFilter("~/test", filter1).type == Found,
        "name mismatch: query = 'test' should match candidate = '~/test'");
  check(applyFilter("/home/you/games/retry.me", filter2).type == Found,
        "name mismatch: query = 'retry.me' should match candidate = "
        "'/home/you/games/retry.me'");
  check(applyFilter("src/main.cpp", filter3).type == Found,
        "name mismatch: query = 'src/main.cpp' should match candidate = "
        "'~/src/main.cpp'");
  check(applyFilter("bak", filter4).type == NotFound,
        "name mismatch: query = 'bak.tar' should not match candidate = 'bak'");
}

std::vector<std::pair<std::string, std::function<void()>>> tests{
    {"name filter validity", test_name_filter_validity},
    {"extension filter validity", test_extension_filter_validity},
    {"type filter validity", test_type_filter_validity},
    {"size filter validity", test_extension_filter_validity},
    {"size parsing", test_size_parsing},
    {"filter sorting", test_sort_filters},
    {"extension filter application test", test_extension_filter},
    {"type filter application test", test_type_filter},
    {"size filter application test", test_size_filter},
    {"name filter application test", test_name_filter},
};

size_t test_count() { return tests.size(); }

int test_all() {
  try {
    create_directory_structure();
  } catch (const std::exception &e) {
    std::cerr << "FAIL: " << "creating directory structure: " << e.what()
              << "\n";
  }

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

  clean_directory_structure();
  return failures;
}

}; // namespace filter
