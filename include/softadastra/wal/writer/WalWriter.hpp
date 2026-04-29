/**
 *
 *  @file WalWriter.hpp
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

#ifndef SOFTADASTRA_WAL_WRITER_HPP
#define SOFTADASTRA_WAL_WRITER_HPP

#include <cstdint>
#include <memory>
#include <utility>

#include <softadastra/core/Core.hpp>
#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/types/FileEventType.hpp>
#include <softadastra/wal/core/Sequence.hpp>
#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/encoding/WalEncoder.hpp>
#include <softadastra/wal/storage/WalStore.hpp>
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/types/WalStatus.hpp>
#include <softadastra/wal/utils/FileEventSerializer.hpp>

namespace softadastra::wal::writer
{
  namespace core = softadastra::wal::core;
  namespace storage = softadastra::wal::storage;
  namespace encoding = softadastra::wal::encoding;
  namespace utils = softadastra::wal::utils;
  namespace wal_types = softadastra::wal::types;
  namespace fs_events = softadastra::fs::events;
  namespace fs_types = softadastra::fs::types;
  namespace core_types = softadastra::core::types;
  namespace core_errors = softadastra::core::errors;

  /**
   * @brief High-level append-only WAL writer.
   *
   * WalWriter is the simple public API used to append durable records to the
   * Write-Ahead Log.
   *
   * It is responsible for:
   * - assigning monotonic sequence numbers
   * - marking records as persisted before encoding
   * - encoding records into the stable WAL format
   * - appending encoded bytes to storage
   * - exposing helpers for filesystem events
   *
   * WalWriter does not replay records.
   * Replay is handled by WalReplayer.
   *
   * Basic usage:
   *
   * @code
   * WalWriter writer{WalConfig::durable("data/wal.log")};
   *
   * auto result = writer.append(
   *     WalRecord::make(
   *         0,
   *         WalRecordType::Put,
   *         payload));
   *
   * if (result.is_ok())
   * {
   *   auto sequence = result.value();
   * }
   * @endcode
   */
  class WalWriter : public core_types::NonCopyable
  {
  public:
    /**
     * @brief Result returned by append operations.
     *
     * The success value is the assigned WAL sequence number.
     */
    using AppendResult =
        core_types::Result<std::uint64_t, core_errors::Error>;

    /**
     * @brief Result returned by flush().
     */
    using Result = core_types::Result<void, core_errors::Error>;

    /**
     * @brief Creates a WAL writer from a configuration.
     *
     * @param config WAL configuration.
     */
    explicit WalWriter(core::WalConfig config)
        : config_(std::move(config)),
          store_(std::make_unique<storage::WalStore>(config_))
    {
    }

    /**
     * @brief Moves a WAL writer.
     */
    WalWriter(WalWriter &&) noexcept = default;

    /**
     * @brief Move-assigns a WAL writer.
     */
    WalWriter &operator=(WalWriter &&) noexcept = default;

    /**
     * @brief Appends a WAL record to durable storage.
     *
     * Flow:
     * - assign a sequence number
     * - mark the record as persisted
     * - encode the record
     * - append bytes to storage
     *
     * If encoding or storage fails, an Error is returned.
     *
     * @param record WAL record to append.
     * @return Assigned sequence number on success.
     */
    [[nodiscard]] AppendResult append(core::WalRecord &record)
    {
      record.sequence = sequence_.next();
      record.timestamp = softadastra::core::time::Timestamp::now();
      record.mark_persisted();

      const auto bytes = encoding::WalEncoder::encode(record);

      if (bytes.empty())
      {
        record.mark_failed();

        return AppendResult::err(
            core_errors::Error::make(
                core_errors::ErrorCode::InvalidArgument,
                "failed to encode WAL record"));
      }

      auto stored = store_->append(bytes);

      if (stored.is_err())
      {
        record.mark_failed();
        return AppendResult::err(stored.error());
      }

      return AppendResult::ok(record.sequence);
    }

    /**
     * @brief Appends a WAL record from type and payload.
     *
     * This is the simplest generic append API.
     *
     * @param type Logical WAL record type.
     * @param payload Opaque binary payload.
     * @return Assigned sequence number on success.
     */
    [[nodiscard]] AppendResult append(
        wal_types::WalRecordType type,
        core::WalRecord::Payload payload)
    {
      core::WalRecord record{
          0,
          type,
          wal_types::WalStatus::Pending,
          softadastra::core::time::Timestamp::now(),
          std::move(payload)};

      return append(record);
    }

    /**
     * @brief Appends a filesystem event to the WAL.
     *
     * The event is serialized into a binary payload and mapped to a generic
     * WAL record type:
     * - Created -> Put
     * - Updated -> Update
     * - Deleted -> Delete
     *
     * @param event Filesystem event to persist.
     * @return Assigned sequence number on success.
     */
    [[nodiscard]] AppendResult append_event(
        const fs_events::FileEvent &event)
    {
      const auto type = map_event_type(event.type);

      if (!wal_types::is_valid(type))
      {
        return AppendResult::err(
            core_errors::Error::make(
                core_errors::ErrorCode::InvalidArgument,
                "invalid filesystem event type"));
      }

      auto payload = utils::FileEventSerializer::serialize(event);

      if (payload.empty())
      {
        return AppendResult::err(
            core_errors::Error::make(
                core_errors::ErrorCode::InvalidArgument,
                "failed to serialize filesystem event"));
      }

      return append(type, std::move(payload));
    }

    /**
     * @brief Flushes the underlying WAL storage.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result flush()
    {
      return store_->flush();
    }

    /**
     * @brief Returns the current sequence value.
     *
     * @return Current WAL sequence.
     */
    [[nodiscard]] std::uint64_t current_sequence() const noexcept
    {
      return sequence_.current();
    }

    /**
     * @brief Restores the sequence after recovery.
     *
     * Use this after reading existing records on disk so the writer continues
     * from the highest known sequence.
     *
     * @param sequence Current sequence value to restore.
     */
    void set_sequence(std::uint64_t sequence) noexcept
    {
      sequence_.set(sequence);
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
      return store_->path();
    }

  private:
    /**
     * @brief Converts a filesystem event type to a WAL record type.
     */
    [[nodiscard]] static constexpr wal_types::WalRecordType
    map_event_type(fs_types::FileEventType type) noexcept
    {
      switch (type)
      {
      case fs_types::FileEventType::Created:
        return wal_types::WalRecordType::Put;

      case fs_types::FileEventType::Updated:
        return wal_types::WalRecordType::Update;

      case fs_types::FileEventType::Deleted:
        return wal_types::WalRecordType::Delete;

      default:
        return wal_types::WalRecordType::Unknown;
      }
    }

  private:
    core::WalConfig config_{};
    core::Sequence sequence_{};
    std::unique_ptr<storage::WalStore> store_{};
  };

} // namespace softadastra::wal::writer

#endif // SOFTADASTRA_WAL_WRITER_HPP
