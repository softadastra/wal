/**
 *
 *  @file WalRecord.hpp
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

#ifndef SOFTADASTRA_WAL_RECORD_HPP
#define SOFTADASTRA_WAL_RECORD_HPP

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <softadastra/core/Core.hpp>
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/types/WalStatus.hpp>

namespace softadastra::wal::core
{
  namespace types = softadastra::wal::types;
  namespace core_time = softadastra::core::time;

  /**
   * @brief Single immutable entry stored in the Write-Ahead Log.
   *
   * WalRecord is the central unit persisted by the WAL.
   *
   * It contains:
   * - a monotonic sequence number
   * - a semantic record type
   * - a lifecycle status
   * - a persisted timestamp
   * - an opaque binary payload
   *
   * The WAL does not interpret the payload.
   * Higher-level modules decide what the bytes represent.
   *
   * Typical payload examples:
   * - serialized filesystem event
   * - sync operation
   * - metadata update
   * - checkpoint marker
   *
   * Rules:
   * - sequence must be greater than zero for persisted records.
   * - type must be valid.
   * - timestamp must be valid.
   * - payload may be empty only for system records such as Noop or Checkpoint.
   */
  struct WalRecord
  {
    /**
     * @brief Binary payload type.
     */
    using Payload = std::vector<std::uint8_t>;

    /**
     * @brief Monotonic sequence number.
     *
     * Used for deterministic ordering during replay.
     */
    std::uint64_t sequence{0};

    /**
     * @brief Logical meaning of the record.
     */
    types::WalRecordType type{types::WalRecordType::Unknown};

    /**
     * @brief Current lifecycle status.
     */
    types::WalStatus status{types::WalStatus::Pending};

    /**
     * @brief Event timestamp in milliseconds since Unix epoch.
     *
     * Timestamp uses Softadastra core::time::Timestamp to keep persisted time
     * stable and easy to serialize.
     */
    core_time::Timestamp timestamp{};

    /**
     * @brief Opaque binary payload.
     *
     * The WAL stores bytes only and does not depend on a specific domain model.
     */
    Payload payload{};

    /**
     * @brief Creates an empty WAL record.
     */
    WalRecord() = default;

    /**
     * @brief Creates a WAL record with all main fields.
     *
     * @param record_sequence Monotonic sequence number.
     * @param record_type Logical record type.
     * @param record_payload Opaque binary payload.
     */
    WalRecord(
        std::uint64_t record_sequence,
        types::WalRecordType record_type,
        Payload record_payload)
        : sequence(record_sequence),
          type(record_type),
          status(types::WalStatus::Pending),
          timestamp(core_time::Timestamp::now()),
          payload(std::move(record_payload))
    {
    }

    /**
     * @brief Creates a WAL record with explicit status and timestamp.
     *
     * Used mainly by decoders and recovery logic.
     *
     * @param record_sequence Monotonic sequence number.
     * @param record_type Logical record type.
     * @param record_status Lifecycle status.
     * @param record_timestamp Persisted timestamp.
     * @param record_payload Opaque binary payload.
     */
    WalRecord(
        std::uint64_t record_sequence,
        types::WalRecordType record_type,
        types::WalStatus record_status,
        core_time::Timestamp record_timestamp,
        Payload record_payload)
        : sequence(record_sequence),
          type(record_type),
          status(record_status),
          timestamp(record_timestamp),
          payload(std::move(record_payload))
    {
    }

    /**
     * @brief Creates a pending WAL record using the current timestamp.
     *
     * @param record_sequence Monotonic sequence number.
     * @param record_type Logical record type.
     * @param record_payload Opaque binary payload.
     * @return Pending WAL record.
     */
    [[nodiscard]] static WalRecord make(
        std::uint64_t record_sequence,
        types::WalRecordType record_type,
        Payload record_payload)
    {
      return WalRecord(
          record_sequence,
          record_type,
          std::move(record_payload));
    }

    /**
     * @brief Returns true if the record contains payload bytes.
     *
     * @return true when payload is not empty.
     */
    [[nodiscard]] bool has_payload() const noexcept
    {
      return !payload.empty();
    }

    /**
     * @brief Returns the payload size in bytes.
     *
     * @return Number of payload bytes.
     */
    [[nodiscard]] std::size_t payload_size() const noexcept
    {
      return payload.size();
    }

    /**
     * @brief Returns true if the record sequence is valid.
     *
     * @return true when sequence is greater than zero.
     */
    [[nodiscard]] bool has_sequence() const noexcept
    {
      return sequence > 0;
    }

    /**
     * @brief Returns true if the record has a valid payload requirement.
     *
     * Data mutation records require a payload.
     * Noop and Checkpoint may have an empty payload.
     *
     * @return true if payload state is valid for this record type.
     */
    [[nodiscard]] bool has_valid_payload() const noexcept
    {
      if (type == types::WalRecordType::Noop ||
          type == types::WalRecordType::Checkpoint)
      {
        return true;
      }

      return has_payload();
    }

    /**
     * @brief Returns true if the record is structurally valid.
     *
     * This does not verify checksums or storage integrity.
     *
     * @return true if core fields are valid.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return has_sequence() &&
             types::is_valid(type) &&
             types::is_valid(status) &&
             timestamp.is_valid() &&
             has_valid_payload();
    }

    /**
     * @brief Marks the record as durably persisted.
     */
    void mark_persisted() noexcept
    {
      status = types::WalStatus::Persisted;
    }

    /**
     * @brief Marks the record as successfully applied.
     */
    void mark_applied() noexcept
    {
      status = types::WalStatus::Applied;
    }

    /**
     * @brief Marks the record as failed.
     */
    void mark_failed() noexcept
    {
      status = types::WalStatus::Failed;
    }

    /**
     * @brief Clears the record and resets it to the default state.
     */
    void clear() noexcept
    {
      sequence = 0;
      type = types::WalRecordType::Unknown;
      status = types::WalStatus::Pending;
      timestamp = core_time::Timestamp{};
      payload.clear();
    }
  };

} // namespace softadastra::wal::core

#endif // SOFTADASTRA_WAL_RECORD_HPP
