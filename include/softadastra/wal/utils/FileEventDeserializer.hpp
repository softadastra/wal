/**
 *
 *  @file FileEventDeserializer.hpp
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

#ifndef SOFTADASTRA_WAL_FILE_EVENT_DESERIALIZER_HPP
#define SOFTADASTRA_WAL_FILE_EVENT_DESERIALIZER_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <softadastra/core/Core.hpp>
#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/state/FileMetadata.hpp>
#include <softadastra/fs/state/FileState.hpp>
#include <softadastra/fs/types/FileEventType.hpp>
#include <softadastra/fs/types/FileType.hpp>

namespace softadastra::wal::utils
{
  namespace fs_events = softadastra::fs::events;
  namespace fs_path = softadastra::fs::path;
  namespace fs_state = softadastra::fs::state;
  namespace fs_types = softadastra::fs::types;
  namespace core_time = softadastra::core::time;

  /**
   * @brief Deserializes filesystem events from WAL payload bytes.
   *
   * FileEventDeserializer reads the binary format produced by
   * FileEventSerializer and reconstructs a FileEvent.
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
   * Invalid, incomplete, unsupported, or corrupted payloads return std::nullopt.
   */
  class FileEventDeserializer
  {
  public:
    /**
     * @brief Supported binary payload format version.
     */
    static constexpr std::uint8_t supported_format_version = 1;

    /**
     * @brief Deserializes a filesystem event from a byte vector.
     *
     * @param data Serialized WAL payload.
     * @return FileEvent on success, std::nullopt on failure.
     */
    [[nodiscard]] static std::optional<fs_events::FileEvent>
    deserialize(const std::vector<std::uint8_t> &data)
    {
      return deserialize(std::span<const std::uint8_t>(data.data(), data.size()));
    }

    /**
     * @brief Deserializes a filesystem event from raw bytes.
     *
     * @param data Pointer to serialized bytes.
     * @param size Number of bytes.
     * @return FileEvent on success, std::nullopt on failure.
     */
    [[nodiscard]] static std::optional<fs_events::FileEvent>
    deserialize(const std::uint8_t *data, std::size_t size)
    {
      if (data == nullptr && size != 0)
      {
        return std::nullopt;
      }

      return deserialize(std::span<const std::uint8_t>(data, size));
    }

    /**
     * @brief Deserializes a filesystem event from a byte span.
     *
     * @param data Serialized WAL payload.
     * @return FileEvent on success, std::nullopt on failure.
     */
    [[nodiscard]] static std::optional<fs_events::FileEvent>
    deserialize(std::span<const std::uint8_t> data)
    {
      Reader reader(data);

      std::uint8_t version = 0;
      if (!reader.read_u8(version))
      {
        return std::nullopt;
      }

      if (version != supported_format_version)
      {
        return std::nullopt;
      }

      std::uint8_t raw_event_type = 0;
      if (!reader.read_u8(raw_event_type))
      {
        return std::nullopt;
      }

      const auto event_type =
          static_cast<fs_types::FileEventType>(raw_event_type);

      if (!is_valid_event_type(event_type))
      {
        return std::nullopt;
      }

      auto current = read_state(reader);
      if (!current.has_value())
      {
        return std::nullopt;
      }

      std::uint8_t has_previous = 0;
      if (!reader.read_u8(has_previous))
      {
        return std::nullopt;
      }

      std::optional<fs_state::FileState> previous = std::nullopt;

      if (has_previous != 0)
      {
        previous = read_state(reader);

        if (!previous.has_value())
        {
          return std::nullopt;
        }
      }

      if (!reader.done())
      {
        return std::nullopt;
      }

      return fs_events::FileEvent{
          event_type,
          std::move(*current),
          std::move(previous)};
    }

  private:
    /**
     * @brief Small bounds-checked binary reader.
     */
    class Reader
    {
    public:
      /**
       * @brief Creates a reader over immutable bytes.
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
       * @brief Reads a length-prefixed string.
       */
      [[nodiscard]] bool read_string(std::string &value)
      {
        std::uint32_t size = 0;

        if (!read_u32(size))
        {
          return false;
        }

        if (!can_read(size))
        {
          return false;
        }

        value.assign(
            reinterpret_cast<const char *>(data_.data() + offset_),
            size);

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

    /**
     * @brief Reads a serialized FileState.
     */
    [[nodiscard]] static std::optional<fs_state::FileState>
    read_state(Reader &reader)
    {
      std::string path_string;

      if (!reader.read_string(path_string))
      {
        return std::nullopt;
      }

      auto path_result = fs_path::Path::from(std::move(path_string));

      if (path_result.is_err())
      {
        return std::nullopt;
      }

      std::uint8_t raw_file_type = 0;
      if (!reader.read_u8(raw_file_type))
      {
        return std::nullopt;
      }

      const auto file_type =
          static_cast<fs_types::FileType>(raw_file_type);

      if (!is_valid_file_type(file_type))
      {
        return std::nullopt;
      }

      std::uint64_t file_size = 0;
      if (!reader.read_u64(file_size))
      {
        return std::nullopt;
      }

      std::int64_t modified_millis = 0;
      if (!reader.read_i64(modified_millis))
      {
        return std::nullopt;
      }

      fs_state::FileMetadata metadata{};
      metadata.type = file_type;
      metadata.size = file_size;
      metadata.modified = core_time::Timestamp::from_millis(modified_millis);

      return fs_state::FileState{
          std::move(path_result.value()),
          metadata,
          std::nullopt};
    }

    /**
     * @brief Returns true if the serialized event type is known.
     */
    [[nodiscard]] static constexpr bool
    is_valid_event_type(fs_types::FileEventType type) noexcept
    {
      return type == fs_types::FileEventType::Created ||
             type == fs_types::FileEventType::Updated ||
             type == fs_types::FileEventType::Deleted;
    }

    /**
     * @brief Returns true if the serialized file type is known.
     */
    [[nodiscard]] static constexpr bool
    is_valid_file_type(fs_types::FileType type) noexcept
    {
      return type == fs_types::FileType::Unknown ||
             type == fs_types::FileType::File ||
             type == fs_types::FileType::Directory ||
             type == fs_types::FileType::Symlink;
    }
  };

} // namespace softadastra::wal::utils

#endif // SOFTADASTRA_WAL_FILE_EVENT_DESERIALIZER_HPP
