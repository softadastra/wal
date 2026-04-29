/**
 *
 *  @file WalReader.hpp
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

#ifndef SOFTADASTRA_WAL_READER_HPP
#define SOFTADASTRA_WAL_READER_HPP

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include <softadastra/core/Core.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/encoding/WalDecoder.hpp>
#include <softadastra/wal/encoding/WalFormat.hpp>

namespace softadastra::wal::reader
{
  namespace core = softadastra::wal::core;
  namespace encoding = softadastra::wal::encoding;
  namespace core_types = softadastra::core::types;
  namespace core_errors = softadastra::core::errors;

  /**
   * @brief Sequential reader for WAL files.
   *
   * WalReader reads encoded WAL records from disk and decodes them one by one.
   *
   * It is designed for:
   * - recovery
   * - replay
   * - diagnostics
   * - tests
   *
   * Behavior:
   * - records are read sequentially
   * - reading stops at the first incomplete or corrupted record
   * - invalid records are not returned
   * - the WAL file is never modified
   *
   * The reader expects records encoded with WalEncoder.
   */
  class WalReader
  {
  public:
    /**
     * @brief Callback invoked for each decoded WAL record.
     */
    using Callback = std::function<void(const core::WalRecord &)>;

    /**
     * @brief Result returned by streaming operations.
     */
    using Result = core_types::Result<void, core_errors::Error>;

    /**
     * @brief Result returned by read_all().
     */
    using RecordsResult =
        core_types::Result<std::vector<core::WalRecord>, core_errors::Error>;

    /**
     * @brief Creates a WAL reader for a file path.
     *
     * @param path WAL file path.
     */
    explicit WalReader(std::string path)
        : path_(std::move(path))
    {
    }

    /**
     * @brief Streams all valid records sequentially.
     *
     * The callback is called once for every decoded record.
     *
     * If the file does not exist or cannot be opened, an error is returned.
     * If corruption or a partial record is detected, reading stops safely and
     * returns success with all records read before the corrupted part.
     *
     * @param callback Function called for each decoded record.
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result for_each(const Callback &callback) const
    {
      std::ifstream in(path_, std::ios::binary);

      if (!in)
      {
        return Result::err(
            core_errors::Error::make(
                core_errors::ErrorCode::FileReadError,
                "failed to open WAL file for reading",
                core_errors::ErrorContext(path_)));
      }

      while (true)
      {
        auto next = read_next(in);

        if (!next.has_value())
        {
          break;
        }

        callback(*next);
      }

      return Result::ok();
    }

    /**
     * @brief Reads all valid WAL records into memory.
     *
     * This is convenient for tests, diagnostics, and small WAL files.
     * For large WAL files, prefer for_each().
     *
     * @return Vector of decoded records on success, Error on failure.
     */
    [[nodiscard]] RecordsResult read_all() const
    {
      std::vector<core::WalRecord> records;

      auto result = for_each(
          [&](const core::WalRecord &record)
          {
            records.push_back(record);
          });

      if (result.is_err())
      {
        return RecordsResult::err(result.error());
      }

      return RecordsResult::ok(std::move(records));
    }

    /**
     * @brief Returns the WAL file path.
     *
     * @return WAL file path.
     */
    [[nodiscard]] const std::string &path() const noexcept
    {
      return path_;
    }

  private:
    /**
     * @brief Reads and decodes the next WAL record from a stream.
     *
     * Returns std::nullopt when:
     * - end of file is reached
     * - a partial record is detected
     * - a corrupted record is detected
     * - an unsupported format version is detected
     *
     * @param in Input stream.
     * @return Decoded WAL record, or std::nullopt.
     */
    [[nodiscard]] static std::optional<core::WalRecord>
    read_next(std::ifstream &in)
    {
      std::vector<std::uint8_t> header(encoding::WalFormat::header_size);

      in.read(
          reinterpret_cast<char *>(header.data()),
          static_cast<std::streamsize>(header.size()));

      const auto header_read = in.gcount();

      if (header_read == 0)
      {
        return std::nullopt;
      }

      if (header_read != static_cast<std::streamsize>(header.size()))
      {
        return std::nullopt;
      }

      const auto payload_size = read_payload_size_from_header(header);

      if (!payload_size.has_value())
      {
        return std::nullopt;
      }

      if (!encoding::WalFormat::valid_payload_size(*payload_size))
      {
        return std::nullopt;
      }

      const std::size_t total_size =
          encoding::WalFormat::header_size +
          static_cast<std::size_t>(*payload_size) +
          encoding::WalFormat::checksum_size;

      std::vector<std::uint8_t> buffer(total_size);

      std::copy(
          header.begin(),
          header.end(),
          buffer.begin());

      const std::size_t remaining =
          total_size - encoding::WalFormat::header_size;

      in.read(
          reinterpret_cast<char *>(
              buffer.data() + encoding::WalFormat::header_size),
          static_cast<std::streamsize>(remaining));

      if (in.gcount() != static_cast<std::streamsize>(remaining))
      {
        return std::nullopt;
      }

      return encoding::WalDecoder::decode(buffer);
    }

    /**
     * @brief Reads payload_size from a WAL header.
     *
     * The payload size field is stored at the end of the header in the v1
     * binary format.
     *
     * @param header WAL header bytes.
     * @return Payload size, or std::nullopt if header is invalid.
     */
    [[nodiscard]] static std::optional<std::uint32_t>
    read_payload_size_from_header(const std::vector<std::uint8_t> &header)
    {
      if (header.size() != encoding::WalFormat::header_size)
      {
        return std::nullopt;
      }

      const std::size_t offset =
          encoding::WalFormat::header_size -
          encoding::WalFormat::payload_size_size;

      std::uint32_t value = 0;

      for (std::uint8_t i = 0; i < 4; ++i)
      {
        value |= static_cast<std::uint32_t>(header[offset + i]) << (i * 8);
      }

      return value;
    }

  private:
    std::string path_{};
  };

} // namespace softadastra::wal::reader

#endif // SOFTADASTRA_WAL_READER_HPP
