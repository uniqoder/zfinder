#include "test_filter.hpp"
#include <iostream>

int main() {
  int total_tests = 0;
  int failures = 0;

  std::cout << "Running tests...\n";

  total_tests += filter::test_count();
  failures += filter::test_all();

  std::cout << "Testing complete. " << total_tests - failures << "/"
            << total_tests << " tests passed.\n";
  return 0;
}
