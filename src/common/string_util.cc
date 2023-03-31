#include "string_util.h"
#include <sstream>
#include <string>

namespace idle {
namespace common {
bool isVisibleChar(char c) { return (c >= 0x20 && c <= 0x7E); }

char toHex(uint8_t i) {
  return i - ((i < 10) ? 1 : 0) * 10 + ((i < 10) ? '0' : 'a');
}

void split(const std::string &str, const std::string &delim,
           std::vector<std::string> *tokens) {
  int lastPos = 0;
  int pos;
  while ((pos = str.find(delim, lastPos)) != std::string::npos) {
    tokens->emplace_back(str.substr(lastPos, pos - lastPos));
    pos = lastPos += delim.length();
  }
  if (pos != str.length()) {
    tokens->emplace_back(str.substr(pos, str.length() - pos));
  }
}
} // namespace common
} // namespace idle