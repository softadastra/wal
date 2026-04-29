/**
 *
 *  @file WalDecoder.hpp
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

#ifndef SOFTADASTRA_WAL_DECODER_HPP
#define SOFTADASTRA_WAL_DECODER_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include <softadastra/core/Core.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/encoding/WalFormat.hpp>
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/types/WalStatus.hpp>
#include <softadastra/wal/utils/Checksum.hpp>

namespace softadastra::wal::encoding
{
  namespace core = softadastra::wal::core;
  namespace types = softadastra::wal::types;
  namespace utils = softadastra::wal::utils;
  namespace core_time = softadastra::core::time;

  /**
   * @brief Decodes WAL records from stable binary bytes.
   *
   * WalDecoder reads the binary format produced by WalEncoder and rebuilds
   * a WalRecord.
   *
   * Binary format version 1:
   *
   * @code
   * uint32 magic
   * uint32 version
   * uint64 sequence
   * uint8  record_type
   * uint8  status
   * int64  timestamp_millis
   * uint32 payload_size
   * bytes  payload
   * uint32 checksum
   * @endcode
   *
   * Integer values are decoded in little-endian order.
   *
   * Checksum rule:
   * - checksum is verified against the payload only
   *
   * Invalid, incomplete, corrupted, or unsupported records return std::nullopt.
   */
  class WalDecoder
  {
  public:
    /**
     * @brief Decodes a WAL record from a byte vector.
     *
     * @param data Encoded WAL bytes.
     * @return Decoded record on success, std::nullopt on failure.
     */
    [[nodiscard]] static std::optional<core::WalRecord>
    decode(const std::vector<std::uint8_t> &data)
    {
      return decode(std::span<const std::uint8_t>(data.data(), data.size()));
    }

    /**
     * @brief Decodes a WAL record from raw bytes.
     *
     * @param data Pointer to encoded bytes.
     * @param size Number of bytes.
     * @return Decoded record on success, std::nullopt on failure.
     */
    [[nodiscard]] static std::optional<core::WalRecord>
    decode(const std::uint8_t *data, std::size_t size)
    {
      if (data == nullptr && size != 0)
      {
        return std::nullopt;
      }

      return decode(std::span<const std::uint8_t>(data, size));
    }

    /**
     * @brief Decodes a WAL record from a byte span.
     *
     * @param data Encoded WAL bytes.
     * @return Decoded record on success, std::nullopt on failure.
     */
    [[nodiscard]] static std::optional<core::WalRecord>
    decode(std::span<const std::uint8_t> data)
    {
      if (data.size() < WalFormat::minimum_record_size)
      {
        return std::nullopt;
      }

      Reader reader(data);

      std::uint32_t magic = 0;
      if (!reader.read_u32(magic) || !WalFormat::valid_magic(magic))
      {
        return std::nullopt;
      }

      std::uint32_t version = 0;
      if (!reader.read_u32(version) || !WalFormat::supported_version(version))
      {
        return std::nullopt;
      }

      std::uint64_t sequence = 0;
      if (!reader.read_u64(sequence))
      {
        return std::nullopt;
      }

      std::uint8_t raw_type = 0;
      if (!reader.read_u8(raw_type))
      {
        return std::nullopt;
      }

      const auto record_type =
          static_cast<types::WalRecordType>(raw_type);

      if (!types::is_valid(record_type))
      {
        return std::nullopt;
      }

      std::uint8_t raw_status = 0;
      if (!reader.read_u8(raw_status))
      {
        return std::nullopt;
      }

      const auto status =
          static_cast<types::WalStatus>(raw_status);

      if (!types::is_valid(status))
      {
        return std::nullopt;
      }

      std::int64_t timestamp_millis = 0;
      if (!reader.read_i64(timestamp_millis))
      {
        return std::nullopt;
      }

      const auto timestamp =
          core_time::Timestamp::from_millis(timestamp_millis);

      std::uint32_t payload_size = 0;
      if (!reader.read_u32(payload_size))
      {
        return std::nullopt;
      }

      if (!WalFormat::valid_payload_size(payload_size))
      {
        return std::nullopt;
      }

      std::vector<std::uint8_t> payload;
      if (!reader.read_bytes(payload, payload_size))
      {
        return std::nullopt;
      }

      std::uint32_t stored_checksum = 0;
      if (!reader.read_u32(stored_checksum))
      {
        return std::nullopt;
      }

      if (!reader.done())
      {
        return std::nullopt;
      }

      if (!utils::Checksum::verify(payload, stored_checksum))
      {
        return std::nullopt;
      }

      core::WalRecord record{
          sequence,
          record_type,
          status,
          timestamp,
          std::move(payload)};

      if (!record.is_valid())
      {
        return std::nullopt;
      }

      return record;
    }

  private:
    /**
     * @brief Bounds-checked little-endian binary reader.
     */
    class Reader
    {
    public:
      /**
       * @brief Creates a reader over immutable bytes.
       *
       * @param data Encoded byte span.
       */
      explicit Reader(std::span<const std::uint8_t> data) noexcept
          : data_(data)
      {
      }

      /**
       * @brief Reads an unsigned 8-bit integer.
       */
      [[nodiscard]] bool read_u8(std::uint8_t &value) noexcept
      {
        if (!can_read(1))
        {
          return false;
        }

        value = data_[offset_];
        ++offset_;
        return true;
      }

      /**
       * @brief Reads an unsigned 32-bit little-endian integer.
       */
      [[nodiscard]] bool read_u32(std::uint32_t &value) noexcept
      {
        if (!can_read(4))
        {
          return false;
        }

        value = 0;

        for (std::uint8_t i = 0; i < 4; ++i)
        {
          value |= static_cast<std::uint32_t>(data_[offset_ + i]) << (i * 8);
        }

        offset_ += 4;
        return true;
      }

      /**
       * @brief Reads an unsigned 64-bit little-endian integer.
       */
      [[nodiscard]] bool read_u64(std::uint64_t &value) noexcept
      {
        if (!can_read(8))
        {
          return false;
        }

        value = 0;

        for (std::uint8_t i = 0; i < 8; ++i)
        {
          value |= static_cast<std::uint64_t>(data_[offset_ + i]) << (i * 8);
        }

        offset_ += 8;
        return true;
      }

      /**
       * @brief Reads a signed 64-bit little-endian integer.
       */
      [[nodiscard]] bool read_i64(std::int64_t &value) noexcept
      {
        std::uint64_t raw = 0;

        if (!read_u64(raw))
        {
          return false;
        }

        value = static_cast<std::int64_t>(raw);
        return true;
      }

      /**
       * @brief Reads a fixed number of bytes into a vector.
       */
      [[nodiscard]] bool read_bytes(
          std::vector<std::uint8_t> &out,
          std::size_t size)
      {
        if (!can_read(size))
        {
          return false;
        }

        out.assign(
            data_.begin() + static_cast<std::ptrdiff_t>(offset_),
            data_.begin() + static_cast<std::ptrdiff_t>(offset_ + size));

        offset_ += size;
        return true;
      }

      /**
       * @brief Returns true when all bytes have been consumed.
       */
      [[nodiscard]] bool done() const noexcept
      {
        return offset_ == data_.size();
      }

    private:
      /**
       * @brief Returns true if count bytes can be read safely.
       */
      [[nodiscard]] bool can_read(std::size_t count) const noexcept
      {
        return count <= data_.size() - offset_;
      }

    private:
      std::span<const std::uint8_t> data_;
      std::size_t offset_{0};
    };
  };

} // namespace softadastra::wal::encoding

#endif // SOFTADASTRA_WAL_DECODER_HPP
