#pragma once

#include <cstdint>
#include <string>

uint32_t edit_distance(std::string query, std::string target);
float normalized_edit_distance(std::string query, std::string target);
