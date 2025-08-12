#ifndef VPT_HASH_HPP
#define VPT_HASH_HPP

#include <bit>
#include <cstdint>
#include <cstddef>

#include <vpt/utils.hpp>

namespace vpt {
namespace detail {

static inline uint64_t MurmurHash64A_padded ( const void * key, int len, uint64_t seed )
{
  constexpr uint64_t m = 0xc6a4a7935bd1e995;
  constexpr int r = 47;

  uint64_t h = seed ^ (len * m);

  const uint64_t * data = (const uint64_t *)key;
  const uint64_t * end = data + (len/8);

  while(data != end)
  {
    static_assert(std::endian::little == std::endian::native);
    uint64_t k = *data++;

    k *= m; 
    k ^= k >> r; 
    k *= m; 
    
    h ^= k;
    h *= m; 
  }

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

}

template <typename... Args>
static inline uint64_t hash(uint64_t seed, Args... args) {
  constexpr size_t args_size = (sizeof(Args) + ...);

  constexpr size_t size = ceildiv<size_t>(args_size, 8) * 8;
  uint64_t key[size];
  key[size-1] = 0;

  char* data = reinterpret_cast<char*>(key);
  ([&](auto arg) {
    memcpy(data, &arg, sizeof(arg));
    data += sizeof(arg);
  }(args), ...);

  return detail::MurmurHash64A_padded(key, size, seed);
}

} // namespace vpt

#endif // !VPT_HASH_HPP