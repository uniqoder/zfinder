#include "fuzzy.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

uint32_t edit_distance(std::string query, std::string target) {
  // 1. Initialize the first row of the matrix
  uint32_t prev[target.length() + 1];
  for (uint32_t i = 0; i < target.length() + 1; i++)
    prev[i] = i;

  // 2. Construct the next row
  uint32_t curr[target.length() + 1];
  for (uint32_t i = 1; i < query.length() + 1; i++) {
    curr[0] = i;

    for (uint32_t j = 1; j < target.length() + 1; j++) {
      uint32_t sub_cost = query[i - 1] != target[j - 1];

      // 3. Calculate the operation with the least cost
      curr[j] = std::min({prev[j] + 1,            // deletion
                          curr[j - 1] + 1,        // insertion
                          prev[j - 1] + sub_cost} // substitution
      );
    }

    memcpy(prev, curr, target.length());
  }

  // 4. Return the last element in the last row of the matrix
  return curr[target.length()];
}

float normalized_edit_distance(std::string query, std::string target) {
  float distance = edit_distance(query, target);
  float accuracy = 1.f - (distance / std::max(query.length(), target.length()));

  // std::cout << "Distance('" << query << "', '" << target << "'): " <<
  // distance
  //           << ", Accuracy: " << std::round(accuracy * 100) << "%\n";

  return accuracy;
}
