/*
 * WalRecord.hpp
 */

#ifndef SOFTADASTRA_WAL_RECORD_HPP
#define SOFTADASTRA_WAL_RECORD_HPP

#include <cstdint>
#include <vector>

#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/types/WalStatus.hpp>

namespace softadastra::wal::core
{
  namespace types = softadastra::wal::types;

  /**
   * @brief WAL record (immutable once persisted)
   *
   * Design:
   * - sequence   → strict monotonic ordering
   * - type       → semantic meaning (Put / Update / Delete / System)
   * - status     → lifecycle state (Pending → Persisted → Applied)
   * - timestamp  → event time (unix seconds)
   * - payload    → opaque binary data (e.g FileEvent serialized)
   *
   * Important:
   * WAL is storage-agnostic.
   * It does NOT know about FileEvent directly.
   * It only stores bytes.
   */
  struct WalRecord
  {
    /**
     * @brief Monotonic sequence number
     */
    std::uint64_t sequence{0};

    /**
     * @brief Logical type of the record
     */
    types::WalRecordType type{types::WalRecordType::Unknown};

    /**
     * @brief Current lifecycle status
     */
    types::WalStatus status{types::WalStatus::Pending};

    /**
     * @brief Timestamp (unix epoch seconds)
     */
    std::uint64_t timestamp{0};

    /**
     * @brief Opaque payload (serialized data)
     */
    std::vector<std::uint8_t> payload;

    /**
     * @brief Check if record has data
     */
    bool has_payload() const noexcept
    {
      return !payload.empty();
    }

    /**
     * @brief Reset record (reuse)
     */
    void clear() noexcept
    {
      sequence = 0;
      type = types::WalRecordType::Unknown;
      status = types::WalStatus::Pending;
      timestamp = 0;
      payload.clear();
    }
  };

} // namespace softadastra::wal::core

#endif
