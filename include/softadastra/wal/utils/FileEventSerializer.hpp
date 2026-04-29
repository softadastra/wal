/**
 *
 *  @file FileEventSerializer.hpp
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

#ifndef SOFTADASTRA_WAL_FILE_EVENT_SERIALIZER_HPP
#define SOFTADASTRA_WAL_FILE_EVENT_SERIALIZER_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <softadastra/core/Core.hpp>
#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/types/FileEventType.hpp>
#include <softadastra/fs/types/FileType.hpp>

namespace softadastra::wal::utils
{
  namespace fs_events = softadastra::fs::events;
  namespace fs_types = softadastra::fs::types;
  namespace core_time = softadastra::core::time;

  /**
   * @brief Serializes filesystem events into stable WAL payload bytes.
   *
   * FileEventSerializer converts a FileEvent into a compact binary payload
   * that can be stored inside a WAL record.
   *
   * Format version 1:
   *
   * @code
   * byte    version
   * byte    event_type
   * string  current_path
   * byte    file_type
   * uint64  file_size
   * int64   modified_millis
   * byte    has_previous
   * string  previous_path        only if has_previous == 1
   * byte    previous_file_type   only if has_previous == 1
   * uint64  previous_file_size   only if has_previous == 1
   * int64   previous_modified    only if has_previous == 1
   * @endcode
   *
   * Strings are encoded as:
   *
   * @code
   * uint32 length
   * bytes  UTF-8/string bytes
   * @endcode
   *
   * Integer values are written in little-endian order to keep the format
   * deterministic across platforms.
   *
   * Notes:
   * - Content hashes are not serialized here yet.
   * - Watcher events may contain partial metadata.
   * - The deserializer must tolerate partial metadata.
   */
  class FileEventSerializer
  {
  public:
    /**
     * @brief Current binary payload format version.
     */
    static constexpr std::uint8_t format_version = 1;

    /**
     * @brief Serializes a filesystem event into bytes.
     *
     * @param event Filesystem event to serialize.
     * @return Binary WAL payload.
     */
    [[nodiscard]] static std::vector<std::uint8_t>
    serialize(const fs_events::FileEvent &event)
    {
      std::vector<std::uint8_t> out;
      out.reserve(64 + event.current.path.str().size());

      append_u8(out, format_version);
      append_u8(out, static_cast<std::uint8_t>(event.type));

      append_state(out, event.current);

      append_u8(out, event.previous.has_value() ? 1U : 0U);

      if (event.previous.has_value())
      {
        append_state(out, *event.previous);
      }

      return out;
    }

  private:
    /**
     * @brief Serializes a FileState.
     */
    static void append_state(
        std::vector<std::uint8_t> &out,
        const softadastra::fs::state::FileState &state)
    {
      append_string(out, state.path.str());
      append_u8(out, static_cast<std::uint8_t>(state.metadata.type));
      append_u64(out, state.metadata.size);
      append_i64(out, state.metadata.modified.millis());
    }

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

    /**
     * @brief Appends a length-prefixed string.
     */
    static void append_string(
        std::vector<std::uint8_t> &out,
        std::string_view value)
    {
      append_u32(out, static_cast<std::uint32_t>(value.size()));

      out.insert(
          out.end(),
          value.begin(),
          value.end());
    }
  };

} // namespace softadastra::wal::utils

#endif // SOFTADASTRA_WAL_FILE_EVENT_SERIALIZER_HPP
