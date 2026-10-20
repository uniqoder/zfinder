#include "argparser.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

void print_help(std::string cmd) {
  std::cout
      << "Usage:\n"
      << cmd
      << " [base path] [arguments]\n\n"
         "arguments:\n"
         "   --name, -n <name>           | Fuzzily matches an item's name.\n"
         "   --type, -t <type>           | Matches an item's type exactly. "
         "Supported values: `f` = file, `d` = directory, `l` = symbolic link.\n"
         "   --extension, -e <extension> | Matches an item's extension "
         "exactly. An empty string matches items without an extension.\n"
         "   --size, -s <size>           | Matches an item's size exactly.\n"
         "   --max-depth, -d <depth>     | Limits the maximum recursion depth. "
         "Depth 0 is the base directory.\n"
         "   --no-threading, -nt         | Disables multithreading during the "
         "search.\n"
         "   --no-color, -nc             | Disables colored output.\n"
         "\nSize prefixes:\n"
         "   K = *10^3\n"
         "   M = *10^6\n"
         "   G = *10^9\n"
         "   T = *10^12\n"
         "   k = *2^10\n"
         "   m = *2^20\n"
         "   g = *2^30\n"
         "   t = *2^40\n"
         "\nSize:\n"
         "   Exact match      - 2G\n"
         "   Less than        - <25M\n"
         "   Less or equal    - <=980\n"
         "   Greater than     - >1m\n"
         "   Greater or equal - >=89\n"
         "   Between          - 23M..2g\n"
         "\nExamples:\n"
         "   zf ./logs --name \"test\" --type f --size \"<=50K\"\n"
         "   zf src -e \"cpp\" -t f\n"
         "   zf . -name \"tests\" -t d\n"
         "   zf foo/bar --size \"24K..1M\"\n"
         "   zf ~/.local/bin --type l --max-depth 0 -nc | uniq | sort\n";
}

bool streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }

Args parse_arguments(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage:\n"
              << argv[0] << " [base directory] [filters] [arguments]\n"
              << "For more help, use the --help or -h argument.\n";
    exit(EXIT_FAILURE);
  }

  if (streq(argv[1], "--help") || streq(argv[1], "-h")) {
    print_help(argv[0]);
    exit(EXIT_SUCCESS);
  }

  if (argc < 4) {
    std::cerr << "Usage:\n"
              << argv[0] << " [base directory] [filters] [arguments]\n"
              << "For more help, use the --help or -h argument.\n";
    exit(EXIT_FAILURE);
  }

  Args args{};
  args.base_dir = argv[1];

  for (int i = 2; i < argc; i++) {
    if (streq(argv[i], "--name") || streq(argv[i], "-n")) {
      if (i >= argc) {
        std::cerr
            << "The name filter takes an argument but none was provided.\n";
        exit(EXIT_FAILURE);
      }

      args.filters.push_back({.type = FilterType::Name, .arg = argv[++i]});
    } else if (streq(argv[i], "--type") || streq(argv[i], "-t")) {
      if (i >= argc) {
        std::cerr
            << "The type filter takes an argument but none was provided.\n";
        exit(EXIT_FAILURE);
      }

      args.filters.push_back({.type = FilterType::Type, .arg = argv[++i]});
    } else if (streq(argv[i], "--extension") || streq(argv[i], "-e")) {
      if (i >= argc) {
        std::cerr << "The extension filter takes an argument but none was "
                     "provided.\n";
        exit(EXIT_FAILURE);
      }

      args.filters.push_back({.type = FilterType::Extension, .arg = argv[++i]});
    } else if (streq(argv[i], "--size") || streq(argv[i], "-s")) {
      if (i >= argc) {
        std::cerr
            << "The size filter takes an argument but none was provided.\n";
        exit(EXIT_FAILURE);
      }

      args.filters.push_back({.type = FilterType::Size, .arg = argv[++i]});
    } else if (streq(argv[i], "--max-depth") || streq(argv[i], "-d")) {
      if (i >= argc) {
        std::cerr
            << "The max-depth argument takes a number but none was provided.\n";
        exit(EXIT_FAILURE);
      }

      uint32_t n = strtoul(argv[++i], nullptr, 10);
      args.max_depth = n;
    } else if (streq(argv[i], "--no-threading") || streq(argv[i], "-nt")) {
      args.do_threading = false;
    } else if (streq(argv[i], "--no-color") || streq(argv[i], "-nc")) {
      args.show_colors = false;
    } else if (streq(argv[i], "--help") || streq(argv[i], "-h")) {
      print_help(argv[0]);
      exit(EXIT_SUCCESS);
    } else {
      std::cout << "Warning: Skipping unknown argument: " << argv[i] << "\n";
    }
  }

  return args;
}
