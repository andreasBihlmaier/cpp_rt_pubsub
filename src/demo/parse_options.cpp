#include "parse_options.h"

#include <iostream>

std::unordered_map<std::string, std::string> parse_options(int argc, char* argv[]) {  // NOLINT
  // TODO(ahb) bad implementation
  std::unordered_map<std::string, std::string> options;
  const size_t option_names_length{5};
  std::array<std::string, option_names_length> option_names{"node_name", "topic_name", "topic_priority",
                                                            "message_type_name", "listen_ip"};
  int i{1};
  while (i < argc) {
    bool option_recognized = false;
    for (auto& option_name : option_names) {
      if (std::string(argv[i]) == "--" + option_name) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        options[option_name] = argv[i + 1];              // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        option_recognized = true;
        i += 2;
        break;
      }
    }
    if (!option_recognized) {
      std::cout << "Unknown option '" << argv[i] << "'" << std::endl;  // NOLINT
      i += 1;
    }
  }
  return options;
}