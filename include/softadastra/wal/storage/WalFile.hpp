/**
 *
 *  @file WalFile.hpp
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

#ifndef SOFTADASTRA_WAL_FILE_HPP
#define SOFTADASTRA_WAL_FILE_HPP

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include <softadastra/core/Core.hpp>

namespace softadastra::wal::storage
{
  namespace core_types = softadastra::core::types;
  namespace core_errors = softadastra::core::errors;

  /**
   * @brief Low-level append-only WAL file wrapper.
   *
   * WalFile owns a single WAL file and provides a small, explicit API for:
   * - opening the file
   * - appending bytes
   * - flushing bytes
   * - reading the whole file
   * - closing the file
   *
   * It does not encode or decode records.
   * Encoding is handled by WalEncoder.
   * Decoding is handled by WalDecoder and WalReader.
   *
   * Rules:
   * - append() writes bytes at the end of the file.
   * - flush() asks the stream to flush buffered bytes.
   * - no exception is thrown by the public API.
   * - failures are returned as Result<T, Error>.
   */
  class WalFile : public core_types::NonCopyable
  {
  public:
    /**
     * @brief Result type for operations without return value.
     */
    using Result = core_types::Result<void, core_errors::Error>;

    /**
     * @brief Result type returned by read_all().
     */
    using BytesResult =
        core_types::Result<std::vector<std::uint8_t>, core_errors::Error>;

    /**
     * @brief Creates a closed WAL file object.
     */
    WalFile() = default;

    /**
     * @brief Creates a WAL file object with a path.
     *
     * The file is not opened automatically.
     * Call open() explicitly.
     *
     * @param path WAL file path.
     */
    explicit WalFile(std::string path)
        : path_(std::move(path))
    {
    }

    /**
     * @brief Closes the WAL file on destruction.
     */
    ~WalFile()
    {
      close();
    }

    WalFile(WalFile &&) noexcept = default;
    WalFile &operator=(WalFile &&) noexcept = default;

    /**
     * @brief Opens the WAL file in append mode.
     *
     * Parent directories are created automatically when needed.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result open()
    {
      if (path_.empty())
      {
        return Result::err(
            core_errors::Error::make(
                core_errors::ErrorCode::InvalidArgument,
                "WAL file path cannot be empty"));
      }

      if (is_open())
      {
        return Result::ok();
      }

      const std::filesystem::path file_path(path_);
      const auto parent = file_path.parent_path();

      if (!parent.empty())
      {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);

        if (ec)
        {
          return Result::err(
              core_errors::Error::make(
                  core_errors::ErrorCode::FileWriteError,
                  "failed to create WAL directory",
                  core_errors::ErrorContext(parent.string())));
        }
      }

      stream_.open(
          path_,
          std::ios::binary |
              std::ios::out |
              std::ios::app);

      if (!stream_)
      {
        return Result::err(
            core_errors::Error::make(
                core_errors::ErrorCode::FileWriteError,
                "failed to open WAL file",
                core_errors::ErrorContext(path_)));
      }

      return Result::ok();
    }

    /**
     * @brief Appends bytes to the WAL file.
     *
     * The file is opened automatically if needed.
     *
     * @param data Bytes to append.
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result append(const std::vector<std::uint8_t> &data)
    {
      if (data.empty())
      {
        return Result::ok();
      }

      auto opened = open();

      if (opened.is_err())
      {
        return opened;
      }

      stream_.write(
          reinterpret_cast<const char *>(data.data()),
          static_cast<std::streamsize>(data.size()));

      if (!stream_)
      {
        return Result::err(
            core_errors::Error::make(
                core_errors::ErrorCode::FileWriteError,
                "failed to append bytes to WAL file",
                core_errors::ErrorContext(path_)));
      }

      return Result::ok();
    }

    /**
     * @brief Flushes buffered WAL bytes to the underlying stream.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result flush()
    {
      if (!is_open())
      {
        return Result::ok();
      }

      stream_.flush();

      if (!stream_)
      {
        return Result::err(
            core_errors::Error::make(
                core_errors::ErrorCode::FileWriteError,
                "failed to flush WAL file",
                core_errors::ErrorContext(path_)));
      }

      return Result::ok();
    }

    /**
     * @brief Reads the entire WAL file into memory.
     *
     * This is useful for tests and diagnostics.
     * For replay, prefer WalReader.
     *
     * @return File bytes on success, Error on failure.
     */
    [[nodiscard]] BytesResult read_all() const
    {
      std::ifstream in(path_, std::ios::binary);

      if (!in)
      {
        return BytesResult::err(
            core_errors::Error::make(
                core_errors::ErrorCode::FileReadError,
                "failed to open WAL file for reading",
                core_errors::ErrorContext(path_)));
      }

      in.seekg(0, std::ios::end);

      const auto end = in.tellg();

      if (end < 0)
      {
        return BytesResult::err(
            core_errors::Error::make(
                core_errors::ErrorCode::FileReadError,
                "failed to determine WAL file size",
                core_errors::ErrorContext(path_)));
      }

      const auto size = static_cast<std::size_t>(end);
      in.seekg(0, std::ios::beg);

      std::vector<std::uint8_t> buffer(size);

      if (size > 0)
      {
        in.read(
            reinterpret_cast<char *>(buffer.data()),
            static_cast<std::streamsize>(size));

        if (!in)
        {
          return BytesResult::err(
              core_errors::Error::make(
                  core_errors::ErrorCode::FileReadError,
                  "failed to read WAL file",
                  core_errors::ErrorContext(path_)));
        }
      }

      return BytesResult::ok(std::move(buffer));
    }

    /**
     * @brief Closes the WAL file.
     */
    void close() noexcept
    {
      if (stream_.is_open())
      {
        stream_.close();
      }
    }

    /**
     * @brief Returns true if the WAL file stream is open.
     *
     * @return true if open.
     */
    [[nodiscard]] bool is_open() const noexcept
    {
      return stream_.is_open();
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

    /**
     * @brief Changes the WAL file path.
     *
     * If the file is already open, it is closed first.
     *
     * @param path New WAL file path.
     */
    void set_path(std::string path)
    {
      close();
      path_ = std::move(path);
    }

  private:
    std::string path_{};
    std::ofstream stream_{};
  };

} // namespace softadastra::wal::storage

#endif // SOFTADASTRA_WAL_FILE_HPP
