#include "filter.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>

// Prefix order matters! Put the longer prefixes at start! Don't forget!!!
const char *SIZE_PREFIXES[] = {"<=", ">=", "<", ">"};
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

char getSizeSuffix(const std::string_view &s) {
  for (const auto &suffix : SIZE_SUFFIXES) {
    if (s.back() == suffix)
      return suffix;
  }

  return 0;
}

SizeType strToSizeType(std::string t) {
  // std::cout << "[?] Converting '" << t << "' to SizeType.\n";
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

SizeStruct parseSize(std::string_view arg) {
  // Handle ranges seperately
  const auto range_sep = arg.find("..");
  if (range_sep != std::string::npos) {
    const auto start = arg.substr(0, range_sep);
    const auto end = arg.substr(range_sep + 2);

    return {.type = SizeType::Range,
            .a = parseSize(start).a,
            .b = parseSize(end).a};
  }

  // Seperate the structure
  const auto prefix = getSizePrefix(arg);
  const auto suffix = getSizeSuffix(arg);

  arg.remove_prefix(prefix.length());
  arg.remove_suffix(suffix != 0);

  int64_t size = strtoll(arg.data(), nullptr, 10);
  int64_t multiple =
      suffix == 0 ? 1l : SIZE_SUFFIX_TO_MULTIPLE[(uint8_t)suffix];
  size *= multiple;

  return {.type = strToSizeType(prefix), .a = size, .b = -1};
}

bool isValidSizeArg(std::string_view arg) {
  const auto size = parseSize(arg);

  if (size.type == SizeType::Range) {
    return size.a != -1 && size.b != -1;
  }

  return size.type != SizeType::Unknown && size.a != -1;
}

bool isFilterValid(const Filter &filter) {
  switch (filter.type) {
  case FilterType::Name:
  case FilterType::Extension:
    return !filter.arg.empty();
  case FilterType::Type:
    return isValidTypeArg(filter.arg);
  case FilterType::Size:
    return isValidSizeArg(filter.arg);
  default:
    return false;
  }
}
