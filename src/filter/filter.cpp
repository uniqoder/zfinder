#include "filter.hpp"
#include "search/fuzzy.hpp"
#include <algorithm>
#include <array>
#include <bits/fs_dir.h>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

// Prefix order matters! Put the longer prefixes at start! Don't forget!!!
const char *SIZE_PREFIXES[] = {"<=", ">=", "<", ">"};
const char SIZE_SUFFIXES[] = {'K', 'k', 'M', 'm', 'G', 'g', 'T', 't'};
const auto SIZE_SUFFIX_TO_MULTIPLE = [] {
  std::array<int64_t, 256> a{};
  a[0] = 1l;
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
  size *= SIZE_SUFFIX_TO_MULTIPLE[(uint8_t)suffix];

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

size_t getSize(fs::path item, fs::file_status fstatus) {
  if (fs::is_symlink(fstatus)) {
    if (fs::is_directory(fstatus)) {
      return fs::file_size(item); // Don't follow symlinks to directories
    }

    item = fs::read_symlink(item);
    fstatus = fs::symlink_status(item);
  }

  if (fs::is_regular_file(fstatus)) {
    return fs::file_size(item);
  }

  if (fs::is_directory(fstatus)) {
    size_t size = 0;
    size_t child_count = 0;
    for (const auto &entry : fs::directory_iterator(item)) {
      child_count++;
      size += getSize(entry, fs::symlink_status(entry));
    }

    return size;
  }

  return 0;
}

std::vector<std::string> split_str(std::string str, char sep) {
  std::vector<std::string> parts;
  // auto max_part_count = std::count(str.begin(), str.end(), '.') * 2;
  // parts.reserve(max_part_count);

  size_t start = 0;
  for (size_t end = 0; end < str.length(); end++) {
    if (str[end] == sep) {
      if (start != end)
        parts.push_back(str.substr(start, end - start));
      start = end + 1;
    }
  }

  if (start != str.length()) // push the last part
    parts.push_back(str.substr(start, str.length() - start));

  return parts;
}

template <typename T> std::string vec_to_str(const std::vector<T> &vec) {
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < vec.size(); i++) {
    oss << vec[i] << (i < vec.size() - 1 ? ", " : "]");
  }

  return oss.str();
}

std::string toLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return str;
}

FilterResult applyNameFilter(const fs::path &item, const std::string &arg) {
  auto target = toLower(item.filename().string());
  auto query = toLower(arg);

  auto target_parts = split_str(target, '.');
  auto query_parts = split_str(query, '.');

  auto acc_sum = 0.f;
  auto acc_count = 0u;
  for (auto qpart : query_parts) {
    // Get the maximum accuracy for this query part between every part of target
    auto max_acc = 0.f;
    for (const auto &tpart : target_parts) {
      auto part_acc = normalized_edit_distance(qpart, tpart);
      if (part_acc > max_acc) {
        max_acc = part_acc;
      }
    }

    // Don't update count if we haven't found a single match yet
    if (true || acc_sum != 0 || max_acc > 0.6f) {
      acc_count++;
      acc_sum += max_acc;
    }
  }

  // Add a penalty if the query and target have different number of parts
  acc_count += std::abs((float)target_parts.size() - query_parts.size());
  auto accuracy = acc_sum / acc_count;

  // Reward for containing the query
  if (target.find(query) != std::string::npos)
    accuracy += 0.4f;
  accuracy = std::clamp(accuracy, 0.f, 1.f);

  return {.type = Found,
          .match_start_idx = 0,
          .match_length = 0,
          .score = accuracy};
}

FilterResult applyExtensionFilter(const fs::path &item,
                                  const std::string &arg) {
  auto filename = item.filename().string();
  auto pos = filename.rfind(arg);

  if (pos != std::string::npos && pos + arg.length() == filename.length())
    return {.type = FilterResultType::Found,
            .match_start_idx = pos,
            .match_length = arg.length(),
            .score = 1.f};
  return {.type = FilterResultType::NotFound,
          .match_start_idx = 0,
          .match_length = 0,
          .score = 0};
}

FilterResult applyTypeFilter(fs::file_status status, const std::string &arg) {
  auto result = [](bool c) {
    return (FilterResult){
        .type = (c ? FilterResultType::Found : FilterResultType::NotFound),
        .match_start_idx = 0,
        .match_length = 0,
        .score = c ? 1.f : 0};
  };

  if (!fs::exists(status))
    return result(false);

  if (arg.back() == 'l')
    return result(fs::is_symlink(status));
  if (arg.back() == 'd')
    return result(fs::is_directory(status));
  if (arg.back() == 'f')
    return result(fs::is_regular_file(status));
  return result(false);
}

FilterResult applySizeFilter(const fs::path &item, const std::string &arg,
                             fs::file_status status) {
  auto result = [](bool c) {
    return (FilterResult){.type = c ? FilterResultType::Found
                                    : FilterResultType::NotFound,
                          .match_start_idx = 0,
                          .match_length = 0,
                          .score = c ? 1.f : 0};
  };

  if (!fs::exists(status)) {
    return result(false);
  }

  int64_t item_size = getSize(item, status);
  auto sizeFilter = parseSize(arg);

  switch (sizeFilter.type) {
  case SizeType::Equal:
    return result(item_size == sizeFilter.a);
  case SizeType::Less:
    return result(item_size < sizeFilter.a);
  case SizeType::LessEqual:
    return result(item_size <= sizeFilter.a);
  case SizeType::More:
    return result(item_size > sizeFilter.a);
  case SizeType::MoreEqual:
    return result(item_size >= sizeFilter.a);
  case SizeType::Range:
    return result(item_size >= sizeFilter.a && item_size < sizeFilter.b);
  default:
    return result(false);
  }
}

FilterResult applyFilter(const fs::path &item, const Filter &filter,
                         fs::file_status fstatus) {
  if (!fs::status_known(fstatus))
    fstatus = fs::symlink_status(item);

  switch (filter.type) {
  case FilterType::Name:
    return applyNameFilter(item, filter.arg);
  case FilterType::Extension:
    return applyExtensionFilter(item, filter.arg);
  case FilterType::Type:
    return applyTypeFilter(fstatus, filter.arg);
  case FilterType::Size:
    return applySizeFilter(item, filter.arg, fstatus);
  default:
    return {
        .type = NotFound, .match_start_idx = 0, .match_length = 0, .score = 0};
  }
}
