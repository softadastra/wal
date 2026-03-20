/*
 * WalWriter.hpp
 */

#ifndef SOFTADASTRA_WAL_WRITER_HPP
#define SOFTADASTRA_WAL_WRITER_HPP

#include <memory>
#include <cstdint>

#include <softadastra/wal/core/Sequence.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/core/WalConfig.hpp>

#include <softadastra/wal/storage/WalStore.hpp>

#include <softadastra/wal/encoding/WalEncoder.hpp>

namespace softadastra::wal::writer
{
  namespace core = softadastra::wal::core;
  namespace storage = softadastra::wal::storage;
  namespace encoding = softadastra::wal::encoding;

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

      // 4. mark as persisted
      record.status = core::types::WalStatus::Persisted;

      return record.sequence;
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
