#include "hash.h"

// FALLTHROUGH_INTENDED is used to explicitly declare we need to
// fall through in switch statement, it does nothing.
#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED \
  do                         \
  {                          \
  } while (0)
#endif

namespace idle
{
  namespace common
  {
    // murmurhash variant
    // this hash function has those features:
    // 1. fast hash, it takes every 4 bytes to do process, it's very fast.
    // 2. use symmetric hash, it does not rely on the order of the data, it does not rely on seed as well.
    // 3. similar to murmur hash, it distributes well.
    uint32_t hash(std::string_view str, size_t n, uint32_t seed)
    {
      const char *data = str.data();
      const uint32_t m = 0xc6a4a793;
      const uint32_t r = 24;
      auto limit = data + n;
      uint32_t h = seed ^ (n * m);

      while (data + 4 <= limit)
      {
        uint32_t w = *reinterpret_cast<const uint32_t *>(data);
        data += 4;
        h += w;
        h *= m;
        h ^= (h >> 16);
      }
      switch (limit - data)
      {
      case 3:
        h += static_cast<unsigned char>(data[2]) << 16;
        FALLTHROUGH_INTENDED;
      case 2:
        h += static_cast<unsigned char>(data[1]) << 8;
        FALLTHROUGH_INTENDED;
      case 1:
        h += static_cast<unsigned char>(data[0]);
        h *= m;
        h ^= (h >> r);
        break;
      }
      return h;
    }
  }
}