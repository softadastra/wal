/**
 *
 *  @file WalReplayer.hpp
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

#ifndef SOFTADASTRA_WAL_REPLAYER_HPP
#define SOFTADASTRA_WAL_REPLAYER_HPP

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>

#include <softadastra/core/Core.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/reader/WalReader.hpp>

namespace softadastra::wal::replay
{
  namespace core = softadastra::wal::core;
  namespace reader = softadastra::wal::reader;
  namespace core_types = softadastra::core::types;
  namespace core_errors = softadastra::core::errors;

  /**
   * @brief Deterministic WAL replay engine.
   *
   * WalReplayer reads records from a WAL file and applies them sequentially.
   *
   * It is used to:
   * - rebuild state after restart
   * - recover pending operations
   * - restore local metadata
   * - feed higher-level sync engines
   *
   * Replay guarantees:
   * - records are processed in WAL order
   * - replay stops safely when the reader stops
   * - corrupted or incomplete records are not applied
   * - last applied sequence is tracked
   *
   * The replayer does not interpret records.
   * The caller provides the apply function.
   */
  class WalReplayer
  {
  public:
    /**
     * @brief Function called for each WAL record.
     */
    using Apply = std::function<void(const core::WalRecord &)>;

    /**
     * @brief Result returned by replay operations.
     */
    using Result = core_types::Result<void, core_errors::Error>;

    /**
     * @brief Creates a replayer from a WAL file path.
     *
     * @param path WAL file path.
     */
    explicit WalReplayer(std::string path)
        : reader_(std::move(path))
    {
    }

    /**
     * @brief Replays all valid WAL records.
     *
     * Records are applied sequentially from the beginning of the WAL.
     *
     * @param apply Function called for each decoded record.
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result replay(const Apply &apply)
    {
      last_sequence_.reset();

      return reader_.for_each(
          [&](const core::WalRecord &record)
          {
            apply(record);
            last_sequence_ = record.sequence;
          });
    }

    /**
     * @brief Replays records starting from a sequence number.
     *
     * Records with sequence lower than start_sequence are skipped.
     *
     * @param start_sequence First sequence number to apply.
     * @param apply Function called for each selected record.
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result replay_from(
        std::uint64_t start_sequence,
        const Apply &apply)
    {
      last_sequence_.reset();

      return reader_.for_each(
          [&](const core::WalRecord &record)
          {
            if (record.sequence < start_sequence)
            {
              return;
            }

            apply(record);
            last_sequence_ = record.sequence;
          });
    }

    /**
     * @brief Returns the last applied sequence number.
     *
     * @return Last applied sequence, or std::nullopt if no record was applied.
     */
    [[nodiscard]] std::optional<std::uint64_t>
    last_sequence() const noexcept
    {
      return last_sequence_;
    }

    /**
     * @brief Returns true if at least one record was applied.
     *
     * @return true when last_sequence() has a value.
     */
    [[nodiscard]] bool has_replayed() const noexcept
    {
      return last_sequence_.has_value();
    }

    /**
     * @brief Returns the underlying WAL file path.
     *
     * @return WAL file path.
     */
    [[nodiscard]] const std::string &path() const noexcept
    {
      return reader_.path();
    }

  private:
    reader::WalReader reader_;
    std::optional<std::uint64_t> last_sequence_{std::nullopt};
  };

} // namespace softadastra::wal::replay

#endif // SOFTADASTRA_WAL_REPLAYER_HPP
