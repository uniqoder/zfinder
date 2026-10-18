#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << argc - 1 << " arguments provided. Command: " << argv[0]
            << std::endl;

  return 0;
}
