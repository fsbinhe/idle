#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string>

namespace idle
{
  namespace common
  {
    uint32_t hash(std::string_view data, size_t n, uint32_t seed);
  }
}