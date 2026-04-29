/**
 *
 *  @file WalRecordType.hpp
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

#ifndef SOFTADASTRA_WAL_RECORD_TYPE_HPP
#define SOFTADASTRA_WAL_RECORD_TYPE_HPP

#include <cstdint>
#include <string_view>

namespace softadastra::wal::types
{
  /**
   * @brief Type of operation stored in the Write-Ahead Log.
   *
   * WalRecordType identifies the semantic meaning of a WAL record.
   *
   * It is used by:
   * - WAL writer
   * - WAL reader
   * - WAL replay engine
   * - sync engine
   * - filesystem event persistence
   *
   * Rules:
   * - Values must remain stable over time.
   * - Do not reorder existing values.
   * - Do not remove existing values once released.
   * - Add new values only at the end.
   * - This enum must stay lightweight and serializable.
   */
  enum class WalRecordType : std::uint8_t
  {
    /**
     * @brief Unknown or invalid record type.
     */
    Unknown = 0,

    /**
     * @brief Creates or inserts a new value.
     *
     * Typical examples:
     * - file created
     * - metadata inserted
     * - object added to the local store
     */
    Put,

    /**
     * @brief Updates an existing value.
     *
     * Typical examples:
     * - file modified
     * - metadata changed
     * - object content updated
     */
    Update,

    /**
     * @brief Deletes an existing value.
     *
     * Typical examples:
     * - file removed
     * - metadata removed
     * - object deleted from the local store
     */
    Delete,

    /**
     * @brief Marks a durable recovery point.
     *
     * A checkpoint can be used by higher-level systems to know that all
     * previous records have been safely applied, compacted, or synchronized.
     */
    Checkpoint,

    /**
     * @brief No-operation record.
     *
     * Useful for testing, padding, heartbeats, or keeping a segment valid
     * without changing application state.
     */
    Noop
  };

  /**
   * @brief Returns a stable string representation of a WAL record type.
   *
   * The returned string is intended for:
   * - logs
   * - debugging
   * - diagnostics
   * - text serialization
   *
   * It is not localized and should not be used as user-facing UI text.
   *
   * @param type WAL record type.
   * @return Stable string representation.
   */
  [[nodiscard]] constexpr std::string_view
  to_string(WalRecordType type) noexcept
  {
    switch (type)
    {
    case WalRecordType::Unknown:
      return "unknown";

    case WalRecordType::Put:
      return "put";

    case WalRecordType::Update:
      return "update";

    case WalRecordType::Delete:
      return "delete";

    case WalRecordType::Checkpoint:
      return "checkpoint";

    case WalRecordType::Noop:
      return "noop";

    default:
      return "invalid";
    }
  }

  /**
   * @brief Returns true if the WAL record type represents a data mutation.
   *
   * Mutation records change application state during replay.
   *
   * @param type WAL record type.
   * @return true for Put, Update, and Delete.
   */
  [[nodiscard]] constexpr bool
  is_mutation(WalRecordType type) noexcept
  {
    return type == WalRecordType::Put ||
           type == WalRecordType::Update ||
           type == WalRecordType::Delete;
  }

  /**
   * @brief Returns true if the WAL record type is valid.
   *
   * Unknown is intentionally treated as invalid.
   *
   * @param type WAL record type.
   * @return true if the type is a known usable record type.
   */
  [[nodiscard]] constexpr bool
  is_valid(WalRecordType type) noexcept
  {
    return type == WalRecordType::Put ||
           type == WalRecordType::Update ||
           type == WalRecordType::Delete ||
           type == WalRecordType::Checkpoint ||
           type == WalRecordType::Noop;
  }

} // namespace softadastra::wal::types

#endif // SOFTADASTRA_WAL_RECORD_TYPE_HPP
