/**
 *
 *  @file WalStatus.hpp
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

#ifndef SOFTADASTRA_WAL_STATUS_HPP
#define SOFTADASTRA_WAL_STATUS_HPP

#include <cstdint>
#include <string_view>

namespace softadastra::wal::types
{

  /**
   * @brief Lifecycle status of a WAL record.
   *
   * WalStatus describes where a record is in the WAL lifecycle.
   *
   * It is used by:
   * - WAL writer
   * - WAL reader
   * - WAL replay engine
   * - recovery logic
   * - sync engine
   *
   * Rules:
   * - Values must remain stable over time.
   * - Do not reorder existing values.
   * - Do not remove existing values once released.
   * - Add new values only at the end.
   * - This enum must stay lightweight and serializable.
   */
  enum class WalStatus : std::uint8_t
  {
    /**
     * @brief Record has been created but not durably persisted yet.
     */
    Pending = 0,

    /**
     * @brief Record has been written to durable storage.
     */
    Persisted,

    /**
     * @brief Record has been successfully replayed or applied.
     */
    Applied,

    /**
     * @brief Record failed during persistence, replay, or application.
     */
    Failed
  };

  /**
   * @brief Returns a stable string representation of a WAL status.
   *
   * The returned string is intended for:
   * - logs
   * - debugging
   * - diagnostics
   * - text serialization
   *
   * It is not localized and should not be used as user-facing UI text.
   *
   * @param status WAL record status.
   * @return Stable string representation.
   */
  [[nodiscard]] constexpr std::string_view
  to_string(WalStatus status) noexcept
  {
    switch (status)
    {
    case WalStatus::Pending:
      return "pending";

    case WalStatus::Persisted:
      return "persisted";

    case WalStatus::Applied:
      return "applied";

    case WalStatus::Failed:
      return "failed";

    default:
      return "invalid";
    }
  }

  /**
   * @brief Returns true if the status is valid.
   *
   * @param status WAL status.
   * @return true if the status is known.
   */
  [[nodiscard]] constexpr bool
  is_valid(WalStatus status) noexcept
  {
    return status == WalStatus::Pending ||
           status == WalStatus::Persisted ||
           status == WalStatus::Applied ||
           status == WalStatus::Failed;
  }

  /**
   * @brief Returns true if the record reached a terminal state.
   *
   * Terminal states mean that the current processing attempt is complete.
   *
   * @param status WAL status.
   * @return true for Applied and Failed.
   */
  [[nodiscard]] constexpr bool
  is_terminal(WalStatus status) noexcept
  {
    return status == WalStatus::Applied ||
           status == WalStatus::Failed;
  }

} // namespace softadastra::wal::types

#endif // SOFTADASTRA_WAL_STATUS_HPP
