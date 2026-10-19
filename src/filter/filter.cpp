#include "filter.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>

enum SizeType { Equal, Less, LessEqual, More, MoreEqual, Range, Unknown };

struct SizeStruct {
  SizeType type;
  int64_t a; // used in most cases when only one value is required.
  int64_t b; // used when two values are required, like in ranges.
};

const char *SIZE_PREFIXES[] = {"<", "<=", ">", ">="};
const char SIZE_SUFFIXES[] = {'K', 'k', 'M', 'm', 'G', 'g', 'T', 't'};
const std::array<int64_t, 256> SIZE_SUFFIX_TO_MULTIPLE = [] {
  std::array<int64_t, 256> a{};
  a['K'] = 1000l;
  a['M'] = 1000l * 1000l;
  a['G'] = 1000l * 1000l * 1000l;
  a['T'] = 1000l * 1000l * 1000l * 1000l;
  a['k'] = 1024l;
  a['m'] = 1024l * 1024l;
  a['g'] = 1024l * 1024l * 1024l;
  a['t'] = 1024l * 1024l * 1024l * 1024l;
  return a;
}();

void sortFilters(std::vector<Filter> &filters) {
  std::sort(filters.begin(), filters.end(),
            [](auto a, auto b) { return a.type < b.type; });
}

bool isValidTypeArg(std::string arg) {
  return arg.length() == 1 && (arg[0] == 'f' || arg[0] == 'd' || arg[0] == 'l');
}

const std::string getSizePrefix(const std::string_view &s) {
  for (const auto &prefix : SIZE_PREFIXES) {
    if (s.find(prefix) == 0)
      return prefix;
  }

  return "";
}

const std::string getSizeSuffix(const std::string_view &s) {
  for (const auto &suffix : SIZE_SUFFIXES) {
    if (s.back() == suffix)
      return std::string(suffix, 1);
  }

  return "";
}

SizeType strToSizeType(std::string t) {
  if (t == "<")
    return SizeType::Less;
  if (t == "<=")
    return SizeType::LessEqual;
  if (t == ">")
    return SizeType::More;
  if (t == ">=")
    return SizeType::MoreEqual;
  if (t.empty())
    return SizeType::Equal;
  return SizeType::Unknown;
}

SizeStruct parseSizeArg(std::string_view arg) {
  // Handle ranges seperately
  const auto range_sep = arg.find("..");
  if (range_sep != std::string::npos) {
    const auto start = arg.substr(0, range_sep);
    const auto end = arg.substr(range_sep + 2);

    return {.type = SizeType::Range,
            .a = parseSizeArg(start).a,
            .b = parseSizeArg(end).a};
  }

  // Seperate the structure
  const auto prefix = getSizePrefix(arg);
  const auto suffix = getSizeSuffix(arg);

  arg.remove_prefix(prefix.length());
  arg.remove_suffix(suffix.length());

  int64_t size = strtoul(arg.data(), nullptr, 10);
  size *= SIZE_SUFFIX_TO_MULTIPLE[(uint8_t)suffix.back()];

  return {.type = strToSizeType(suffix), .a = size, .b = -1};
}

bool isValidSizeArg(std::string_view arg) {
  const auto size = parseSizeArg(arg);

  if (size.type == SizeType::Range) {
    return size.a != -1 && size.b != -1;
  }

  return size.type != SizeType::Unknown && size.a != -1;
}

bool isFilterValid(const Filter &filter) {
  switch (filter.type) {
  case FilterType::Name:
  case FilterType::Extension:
    return true;
  case FilterType::Type:
    return isValidTypeArg(filter.arg);
  case FilterType::Size:
    return isValidSizeArg(filter.arg);
  default:
    return false;
  }
}
