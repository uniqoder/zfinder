#include "fuzzy.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

uint32_t edit_distance(std::string query, std::string target) {
  uint32_t row[target.length() + 1];
  for (uint32_t i = 0; i < target.length() + 1; i++)
    row[i] = i;

  for (uint32_t i = 1; i < query.length() + 1; i++) {
    auto corner = row[0];
    row[0] = i;

    for (uint32_t j = 1; j < target.length() + 1; j++) {
      uint32_t cost = query[i - 1] != target[j - 1];

      auto op = std::min({row[j] + 1,     // deletion
                          row[j - 1] + 1, // insertion
                          corner + cost}  // substitution
      );

      corner = row[j];
      row[j] = op;
    }
  }

  // 4. Return the last element in the last row of the matrix
  return row[target.length()];
}

float normalized_edit_distance(std::string query, std::string target) {
  float distance = edit_distance(query, target);
  float accuracy = 1.f - (distance / std::max(query.length(), target.length()));

  return accuracy;
}
