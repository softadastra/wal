/**
 *
 *  @file WalFormat.hpp
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

#ifndef SOFTADASTRA_WAL_FORMAT_HPP
#define SOFTADASTRA_WAL_FORMAT_HPP

#include <cstddef>
#include <cstdint>

namespace softadastra::wal::encoding
{
  /**
   * @brief Stable binary format constants for WAL records.
   *
   * WalFormat defines the low-level binary layout used by WalEncoder and
   * WalDecoder.
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
   * - checksum does not include header fields
   *
   * Rules:
   * - Do not change existing field order.
   * - Do not change existing field sizes.
   * - Add future fields only by increasing version.
   */
  struct WalFormat
  {
    /**
     * @brief WAL magic value.
     *
     * ASCII representation: "WAL1".
     */
    static constexpr std::uint32_t magic = 0x57414C31u;

    /**
     * @brief Current WAL binary format version.
     */
    static constexpr std::uint32_t version = 1u;

    /**
     * @brief Serialized size of the magic field.
     */
    static constexpr std::size_t magic_size = sizeof(std::uint32_t);

    /**
     * @brief Serialized size of the version field.
     */
    static constexpr std::size_t version_size = sizeof(std::uint32_t);

    /**
     * @brief Serialized size of the sequence field.
     */
    static constexpr std::size_t sequence_size = sizeof(std::uint64_t);

    /**
     * @brief Serialized size of the record type field.
     */
    static constexpr std::size_t record_type_size = sizeof(std::uint8_t);

    /**
     * @brief Serialized size of the record status field.
     */
    static constexpr std::size_t status_size = sizeof(std::uint8_t);

    /**
     * @brief Serialized size of the timestamp field.
     */
    static constexpr std::size_t timestamp_size = sizeof(std::int64_t);

    /**
     * @brief Serialized size of the payload size field.
     */
    static constexpr std::size_t payload_size_size = sizeof(std::uint32_t);

    /**
     * @brief Serialized size of the checksum field.
     */
    static constexpr std::size_t checksum_size = sizeof(std::uint32_t);

    /**
     * @brief Serialized size of a WAL record header.
     *
     * This does not include payload bytes or checksum bytes.
     */
    static constexpr std::size_t header_size =
        magic_size +
        version_size +
        sequence_size +
        record_type_size +
        status_size +
        timestamp_size +
        payload_size_size;

    /**
     * @brief Minimum serialized record size.
     *
     * This includes the header and checksum, but excludes payload bytes.
     */
    static constexpr std::size_t minimum_record_size =
        header_size + checksum_size;

    /**
     * @brief Maximum payload size accepted by the default WAL format.
     *
     * 64 MiB is large enough for filesystem metadata/events, while still
     * protecting the decoder from corrupted size fields.
     */
    static constexpr std::uint32_t max_payload_size =
        64u * 1024u * 1024u;

    /**
     * @brief Returns true if a magic value matches the WAL format.
     *
     * @param value Magic value to check.
     * @return true if value is the expected WAL magic.
     */
    [[nodiscard]] static constexpr bool valid_magic(
        std::uint32_t value) noexcept
    {
      return value == magic;
    }

    /**
     * @brief Returns true if a version is supported by this decoder.
     *
     * @param value Version to check.
     * @return true if value is supported.
     */
    [[nodiscard]] static constexpr bool supported_version(
        std::uint32_t value) noexcept
    {
      return value == version;
    }

    /**
     * @brief Returns true if a payload size is accepted by the format.
     *
     * @param value Payload size in bytes.
     * @return true if the payload size is within the supported limit.
     */
    [[nodiscard]] static constexpr bool valid_payload_size(
        std::uint32_t value) noexcept
    {
      return value <= max_payload_size;
    }
  };

} // namespace softadastra::wal::encoding

#endif // SOFTADASTRA_WAL_FORMAT_HPP
