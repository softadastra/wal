/*
 * WalWriter.hpp
 */

#ifndef SOFTADASTRA_WAL_WRITER_HPP
#define SOFTADASTRA_WAL_WRITER_HPP

#include <memory>
#include <cstdint>
#include <ctime>

#include <softadastra/wal/core/Sequence.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/storage/WalStore.hpp>
#include <softadastra/wal/encoding/WalEncoder.hpp>
#include <softadastra/wal/utils/FileEventSerializer.hpp>
#include <softadastra/fs/events/FileEvent.hpp>

namespace softadastra::wal::writer
{
  namespace core = softadastra::wal::core;
  namespace storage = softadastra::wal::storage;
  namespace encoding = softadastra::wal::encoding;
  namespace utils = softadastra::wal::utils;
  namespace fs_events = softadastra::fs::events;

  class WalWriter
  {
  public:
    explicit WalWriter(const core::WalConfig &config)
        : config_(config),
          store_(std::make_unique<storage::WalStore>(config))
    {
    }

    /**
     * @brief Append a record to WAL (safe)
     *
     * Flow:
     * 1. assign sequence
     * 2. encode
     * 3. append
     * 4. flush (if enabled)
     */
    std::uint64_t append(core::WalRecord &record)
    {
      // 1. assign sequence
      record.sequence = sequence_.next();

      // 2. encode
      auto bytes = encoding::WalEncoder::encode(record);

      // 3. append to storage
      store_->append(bytes);

      // 4. flush if enabled
      if (config_.auto_flush)
      {
        store_->flush();
      }

      // 5. mark as persisted
      record.status = core::types::WalStatus::Persisted;

      return record.sequence;
    }

    /**
     * @brief Append a filesystem event to WAL
     */
    std::uint64_t append_event(const fs_events::FileEvent &event)
    {
      core::WalRecord record;

      switch (event.type)
      {
      case softadastra::fs::types::FileEventType::Created:
        record.type = core::types::WalRecordType::Put;
        break;

      case softadastra::fs::types::FileEventType::Updated:
        record.type = core::types::WalRecordType::Update;
        break;

      case softadastra::fs::types::FileEventType::Deleted:
        record.type = core::types::WalRecordType::Delete;
        break;

      default:
        record.type = core::types::WalRecordType::Unknown;
        break;
      }

      record.timestamp = static_cast<std::uint64_t>(std::time(nullptr));
      record.payload = utils::FileEventSerializer::serialize(event);

      return append(record);
    }

    /**
     * @brief Force flush WAL
     */
    void flush()
    {
      store_->flush();
    }

  private:
    core::WalConfig config_;
    core::Sequence sequence_;
    std::unique_ptr<storage::WalStore> store_;
  };

} // namespace softadastra::wal::writer

#endif
