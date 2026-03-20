/*
 * Checksum.hpp
 */

#ifndef SOFTADASTRA_WAL_CHECKSUM_HPP
#define SOFTADASTRA_WAL_CHECKSUM_HPP

#include <cstdint>
#include <cstddef>

namespace softadastra::wal::utils
{

  /**
   * @brief Checksum utilities for WAL
   *
   * Uses FNV-1a 32-bit:
   * - fast
   * - deterministic
   * - no dependencies
   */
  struct Checksum
  {
    /**
     * @brief Compute checksum for a buffer
     */
    static std::uint32_t compute(const std::uint8_t *data,
                                 std::size_t size) noexcept
    {
      std::uint32_t hash = 2166136261u;

      for (std::size_t i = 0; i < size; ++i)
      {
        hash ^= data[i];
        hash *= 16777619u;
      }

      return hash;
    }

    /**
     * @brief Verify checksum
     */
    static bool verify(const std::uint8_t *data,
                       std::size_t size,
                       std::uint32_t expected) noexcept
    {
      return compute(data, size) == expected;
    }
  };

} // namespace softadastra::wal::utils

#endif
