/**
 *
 *  @file WalConfig.hpp
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

#ifndef SOFTADASTRA_WAL_CONFIG_HPP
#define SOFTADASTRA_WAL_CONFIG_HPP

#include <cstddef>
#include <string>

namespace softadastra::wal::core
{

  /**
   * @brief Configuration used by WAL readers, writers, and stores.
   *
   * WalConfig keeps the public WAL setup intentionally small.
   *
   * It defines:
   * - where records are stored
   * - whether writes are flushed immediately
   * - the maximum segment size
   * - whether payload checksums are enabled
   *
   * This struct is designed to be simple to construct, copy, serialize,
   * and pass across higher-level Softadastra modules.
   */
  struct WalConfig
  {
    /**
     * @brief Path to the WAL file or active WAL segment.
     *
     * For the simple API, this is the only required configuration value.
     */
    std::string path{"data/wal.log"};

    /**
     * @brief Flush the WAL after every append.
     *
     * When true, each append asks the underlying storage to flush data.
     * This improves durability but may reduce throughput.
     */
    bool auto_flush{true};

    /**
     * @brief Maximum WAL file size before rotation.
     *
     * The default value is 64 MiB.
     *
     * Rotation can be implemented by WalStore or segment-based storage.
     */
    std::size_t max_file_size{64u * 1024u * 1024u};

    /**
     * @brief Enable payload checksum verification.
     *
     * When true, encoded records include and verify a checksum for payload
     * corruption detection.
     */
    bool enable_checksum{true};

    /**
     * @brief Creates a default WAL configuration.
     */
    WalConfig() = default;

    /**
     * @brief Creates a WAL configuration with a custom path.
     *
     * @param wal_path WAL file path.
     */
    explicit WalConfig(std::string wal_path)
        : path(std::move(wal_path))
    {
    }

    /**
     * @brief Returns a production-oriented default configuration.
     *
     * @param wal_path WAL file path.
     * @return WAL configuration with durable defaults.
     */
    [[nodiscard]] static WalConfig durable(std::string wal_path)
    {
      WalConfig config(std::move(wal_path));
      config.auto_flush = true;
      config.enable_checksum = true;
      config.max_file_size = 64u * 1024u * 1024u;
      return config;
    }

    /**
     * @brief Returns a faster configuration for tests or benchmarks.
     *
     * @param wal_path WAL file path.
     * @return WAL configuration optimized for lower write overhead.
     */
    [[nodiscard]] static WalConfig fast(std::string wal_path)
    {
      WalConfig config(std::move(wal_path));
      config.auto_flush = false;
      config.enable_checksum = true;
      config.max_file_size = 64u * 1024u * 1024u;
      return config;
    }

    /**
     * @brief Returns true if the configuration looks usable.
     *
     * @return true when path is not empty and max_file_size is non-zero.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return !path.empty() && max_file_size > 0;
    }
  };

} // namespace softadastra::wal::core

#endif // SOFTADASTRA_WAL_CONFIG_HPP
