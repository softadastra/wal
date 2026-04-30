/**
 *
 *  @file Checksum.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra WAL
 *
 */

#ifndef SOFTADASTRA_WAL_CHECKSUM_HPP
#define SOFTADASTRA_WAL_CHECKSUM_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace softadastra::wal::utils
{
  /**
   * @brief Small deterministic checksum utility for WAL records.
   *
   * Checksum computes and verifies a lightweight checksum used to detect
   * corrupted WAL payloads during reading and replay.
   *
   * The current implementation uses FNV-1a 32-bit because it is:
   * - fast
   * - deterministic
   * - dependency-free
   * - stable across platforms
   *
   * Important:
   * - This is not a cryptographic hash.
   * - It must not be used for security.
   * - It is only intended for corruption detection.
   */
  class Checksum
  {
  public:
    /**
     * @brief Underlying checksum value type.
     */
    using value_type = std::uint32_t;

    /**
     * @brief Computes a checksum from a raw byte buffer.
     *
     * Passing a null pointer with a non-zero size returns 0.
     *
     * @param data Pointer to the first byte.
     * @param size Number of bytes to read.
     * @return Computed checksum value.
     */
    [[nodiscard]] static constexpr value_type compute(
        const std::uint8_t *data,
        std::size_t size) noexcept
    {
      if (data == nullptr && size != 0)
      {
        return 0;
      }

      value_type hash = offset_basis();

      for (std::size_t i = 0; i < size; ++i)
      {
        hash ^= data[i];
        hash *= prime();
      }

      return hash;
    }

    /**
     * @brief Computes a checksum from a byte span.
     *
     * @param data Bytes to checksum.
     * @return Computed checksum value.
     */
    [[nodiscard]] static constexpr value_type compute(
        std::span<const std::uint8_t> data) noexcept
    {
      return compute(data.data(), data.size());
    }

    /**
     * @brief Computes a checksum from a byte vector.
     *
     * @param data Bytes to checksum.
     * @return Computed checksum value.
     */
    [[nodiscard]] static value_type compute(
        const std::vector<std::uint8_t> &data) noexcept
    {
      return compute(data.data(), data.size());
    }

    /**
     * @brief Computes a checksum from a string view.
     *
     * This overload treats the string as raw bytes.
     *
     * @param data Text payload.
     * @return Computed checksum value.
     */
    [[nodiscard]] static constexpr value_type compute(
        std::string_view data) noexcept
    {
      value_type hash = offset_basis();

      for (char c : data)
      {
        hash ^= static_cast<std::uint8_t>(c);
        hash *= prime();
      }

      return hash;
    }

    /**
     * @brief Verifies a checksum for a raw byte buffer.
     *
     * @param data Pointer to the first byte.
     * @param size Number of bytes to read.
     * @param expected Expected checksum value.
     * @return true if computed checksum equals expected.
     */
    [[nodiscard]] static constexpr bool verify(
        const std::uint8_t *data,
        std::size_t size,
        value_type expected) noexcept
    {
      return compute(data, size) == expected;
    }

    /**
     * @brief Verifies a checksum for a byte span.
     *
     * @param data Bytes to verify.
     * @param expected Expected checksum value.
     * @return true if computed checksum equals expected.
     */
    [[nodiscard]] static constexpr bool verify(
        std::span<const std::uint8_t> data,
        value_type expected) noexcept
    {
      return compute(data) == expected;
    }

    /**
     * @brief Verifies a checksum for a byte vector.
     *
     * @param data Bytes to verify.
     * @param expected Expected checksum value.
     * @return true if computed checksum equals expected.
     */
    [[nodiscard]] static bool verify(
        const std::vector<std::uint8_t> &data,
        value_type expected) noexcept
    {
      return compute(data) == expected;
    }

    /**
     * @brief Verifies a checksum for a string view.
     *
     * @param data Text payload.
     * @param expected Expected checksum value.
     * @return true if computed checksum equals expected.
     */
    [[nodiscard]] static constexpr bool verify(
        std::string_view data,
        value_type expected) noexcept
    {
      return compute(data) == expected;
    }

  private:
    /**
     * @brief FNV-1a 32-bit offset basis.
     */
    [[nodiscard]] static constexpr value_type offset_basis() noexcept
    {
      return 2166136261u;
    }

    /**
     * @brief FNV-1a 32-bit prime.
     */
    [[nodiscard]] static constexpr value_type prime() noexcept
    {
      return 16777619u;
    }
  };

} // namespace softadastra::wal::utils

#endif // SOFTADASTRA_WAL_CHECKSUM_HPP
