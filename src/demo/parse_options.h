#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> parse_options(int argc, char* argv[]);  // NOLINT

#endif  // PARSE_OPTIONS_H