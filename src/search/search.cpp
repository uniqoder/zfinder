#include "search.hpp"
#include "filter/filter.hpp"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <vector>

void sortSearchResults(std::vector<SearchResult> &results) {
  std::sort(results.begin(), results.end(),
            [](auto a, auto b) { return a.accuracy > b.accuracy; });
}

std::vector<SearchResult>
performSearch(const fs::path &root, const std::vector<Filter> &filters,
              uint32_t max_depth,
              const std::function<void(const SearchResult &)> &onResult) {
  std::vector<SearchResult> results;

  bool isMatch;
  bool accAccumulationStarted;
  SearchResult result;
  result.accuracy = 0;
  for (const auto &entry : fs::directory_iterator(root)) {
    isMatch = true;
    accAccumulationStarted = false;
    result.match_starts.clear();
    result.match_ends.clear();
    result.item_path = entry.path();
    result.accuracy = 0;

    fs::file_status fstatus = entry.symlink_status();

    for (const auto &filter : filters) {
      auto fresult = applyFilter(result.item_path, filter, fstatus);
      if (fresult.type == FilterResultType::Found) {
        result.accuracy += fresult.score;
        if (accAccumulationStarted)
          result.accuracy /= 2.f;
        else
          accAccumulationStarted = true;

        if (result.accuracy < 0.5f) {
          isMatch = false;
          break;
        }

        if (std::max(fresult.match_length, fresult.match_start_idx) == 0)
          continue;

        result.match_starts.push_back(fresult.match_start_idx);
        result.match_ends.push_back(fresult.match_start_idx +
                                    fresult.match_length);
      } else {
        isMatch = false;
        break;
      }
    }

    if (isMatch) {
      results.push_back(result);
      onResult(result);
    }

    if (fs::is_directory(fstatus) && max_depth > 0) {
      auto child_results =
          performSearch(result.item_path, filters, max_depth - 1, onResult);
      results.reserve(results.size() + child_results.size());
      results.insert(results.end(), child_results.begin(), child_results.end());
    }
  }

  return results;
}
