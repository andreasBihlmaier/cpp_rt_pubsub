#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> parse_options(int argc, char* argv[]);  // NOLINT

std::string option_or_default(const std::unordered_map<std::string, std::string>& p_options,
                              const std::string& p_option_name, const std::string& p_default_value);

#endif  // PARSE_OPTIONS_H