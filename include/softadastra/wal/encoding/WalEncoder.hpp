/**
 *
 *  @file WalEncoder.hpp
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

#ifndef SOFTADASTRA_WAL_ENCODER_HPP
#define SOFTADASTRA_WAL_ENCODER_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/encoding/WalFormat.hpp>
#include <softadastra/wal/utils/Checksum.hpp>

namespace softadastra::wal::encoding
{
  namespace core = softadastra::wal::core;
  namespace utils = softadastra::wal::utils;

  /**
   * @brief Encodes WAL records into stable binary bytes.
   *
   * WalEncoder converts a WalRecord into the binary format defined by
   * WalFormat.
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
   * Integer values are encoded in little-endian order.
   *
   * Checksum rule:
   * - checksum is computed over the payload only
   * - checksum is written after the payload
   *
   * Invalid records return an empty buffer.
   */
  class WalEncoder
  {
  public:
    /**
     * @brief Encodes a WAL record into bytes.
     *
     * @param record WAL record to encode.
     * @return Encoded binary record, or an empty vector if the record is invalid.
     */
    [[nodiscard]] static std::vector<std::uint8_t>
    encode(const core::WalRecord &record)
    {
      if (!can_encode(record))
      {
        return {};
      }

      const auto payload_size =
          static_cast<std::uint32_t>(record.payload.size());

      const std::size_t total_size =
          WalFormat::header_size +
          record.payload.size() +
          WalFormat::checksum_size;

      std::vector<std::uint8_t> out;
      out.reserve(total_size);

      append_u32(out, WalFormat::magic);
      append_u32(out, WalFormat::version);
      append_u64(out, record.sequence);
      append_u8(out, static_cast<std::uint8_t>(record.type));
      append_u8(out, static_cast<std::uint8_t>(record.status));
      append_i64(out, record.timestamp.millis());
      append_u32(out, payload_size);

      out.insert(
          out.end(),
          record.payload.begin(),
          record.payload.end());

      const auto checksum = utils::Checksum::compute(record.payload);
      append_u32(out, checksum);

      return out;
    }

    /**
     * @brief Returns true if a record can be encoded safely.
     *
     * @param record WAL record to validate.
     * @return true if the record is structurally valid and payload size is supported.
     */
    [[nodiscard]] static bool can_encode(
        const core::WalRecord &record) noexcept
    {
      if (!record.is_valid())
      {
        return false;
      }

      if (record.payload.size() > WalFormat::max_payload_size)
      {
        return false;
      }

      return true;
    }

  private:
    /**
     * @brief Appends one unsigned byte.
     */
    static void append_u8(
        std::vector<std::uint8_t> &out,
        std::uint8_t value)
    {
      out.push_back(value);
    }

    /**
     * @brief Appends a 32-bit unsigned integer in little-endian order.
     */
    static void append_u32(
        std::vector<std::uint8_t> &out,
        std::uint32_t value)
    {
      for (std::uint8_t i = 0; i < 4; ++i)
      {
        out.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFFU));
      }
    }

    /**
     * @brief Appends a 64-bit unsigned integer in little-endian order.
     */
    static void append_u64(
        std::vector<std::uint8_t> &out,
        std::uint64_t value)
    {
      for (std::uint8_t i = 0; i < 8; ++i)
      {
        out.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFFU));
      }
    }

    /**
     * @brief Appends a 64-bit signed integer in little-endian order.
     */
    static void append_i64(
        std::vector<std::uint8_t> &out,
        std::int64_t value)
    {
      append_u64(out, static_cast<std::uint64_t>(value));
    }
  };

} // namespace softadastra::wal::encoding

#endif // SOFTADASTRA_WAL_ENCODER_HPP
