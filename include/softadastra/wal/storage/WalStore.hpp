/**
 *
 *  @file WalStore.hpp
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

#ifndef SOFTADASTRA_WAL_STORE_HPP
#define SOFTADASTRA_WAL_STORE_HPP

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <softadastra/core/Core.hpp>
#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/storage/WalFile.hpp>

namespace softadastra::wal::storage
{
  namespace core = softadastra::wal::core;
  namespace core_types = softadastra::core::types;
  namespace core_errors = softadastra::core::errors;

  /**
   * @brief Simple durable storage layer for WAL bytes.
   *
   * WalStore is the storage facade used by WalWriter.
   *
   * It is responsible for:
   * - owning the active WAL file
   * - appending encoded record bytes
   * - flushing when configured
   * - exposing read_all() for diagnostics and tests
   *
   * WalStore does not know how to encode or decode WAL records.
   * It only stores bytes.
   *
   * For now, WalStore uses a single append-only file.
   * Segment rotation can be added later without changing WalWriter's public API.
   */
  class WalStore : public core_types::NonCopyable
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
     * @brief Creates a WAL store from a config.
     *
     * The underlying file is not opened immediately.
     * It is opened lazily on first append().
     *
     * @param config WAL configuration.
     */
    explicit WalStore(core::WalConfig config)
        : config_(std::move(config)),
          file_(std::make_unique<WalFile>(config_.path))
    {
    }

    /**
     * @brief Moves a WAL store.
     */
    WalStore(WalStore &&) noexcept = default;

    /**
     * @brief Move-assigns a WAL store.
     */
    WalStore &operator=(WalStore &&) noexcept = default;

    /**
     * @brief Appends encoded WAL bytes to storage.
     *
     * If config.auto_flush is true, the store flushes after a successful append.
     *
     * @param data Encoded WAL record bytes.
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result append(const std::vector<std::uint8_t> &data)
    {
      if (data.empty())
      {
        return Result::ok();
      }

      auto result = file_->append(data);

      if (result.is_err())
      {
        return result;
      }

      bytes_written_ += static_cast<std::uint64_t>(data.size());

      if (config_.auto_flush)
      {
        return file_->flush();
      }

      return Result::ok();
    }

    /**
     * @brief Flushes the underlying WAL storage.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result flush()
    {
      return file_->flush();
    }

    /**
     * @brief Reads the full WAL file into memory.
     *
     * This is intended for tests and diagnostics.
     * For recovery and replay, prefer WalReader.
     *
     * @return File bytes on success, Error on failure.
     */
    [[nodiscard]] BytesResult read_all() const
    {
      return file_->read_all();
    }

    /**
     * @brief Returns true if the underlying file is open.
     *
     * @return true if open.
     */
    [[nodiscard]] bool is_open() const noexcept
    {
      return file_ && file_->is_open();
    }

    /**
     * @brief Returns the number of bytes written through this store instance.
     *
     * This counter starts at zero when WalStore is created.
     * It does not scan existing bytes already present on disk.
     *
     * @return Number of bytes appended by this store instance.
     */
    [[nodiscard]] std::uint64_t bytes_written() const noexcept
    {
      return bytes_written_;
    }

    /**
     * @brief Returns the WAL configuration.
     *
     * @return WAL configuration.
     */
    [[nodiscard]] const core::WalConfig &config() const noexcept
    {
      return config_;
    }

    /**
     * @brief Returns the WAL file path.
     *
     * @return WAL file path.
     */
    [[nodiscard]] const std::string &path() const noexcept
    {
      return config_.path;
    }

  private:
    core::WalConfig config_{};
    std::unique_ptr<WalFile> file_{};
    std::uint64_t bytes_written_{0};
  };

} // namespace softadastra::wal::storage

#endif // SOFTADASTRA_WAL_STORE_HPP
