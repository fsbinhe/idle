#pragma once

#include <cstdint>
#include <string>
#include <vector>
namespace idle {
namespace common {
bool isVisibleChar(char c);

char toHex(uint8_t i);

void split(const std::string &str, const std::string &delim,
           std::vector<std::string> *tokens);

std::string trim(const std::string &str, const std::string &trim);
} // namespace common
} // namespace idle