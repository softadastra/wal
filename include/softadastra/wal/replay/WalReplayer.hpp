/*
 * WalReplayer.hpp
 */

#ifndef SOFTADASTRA_WAL_REPLAYER_HPP
#define SOFTADASTRA_WAL_REPLAYER_HPP

#include <functional>
#include <cstdint>
#include <optional>

#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/reader/WalReader.hpp>

namespace softadastra::wal::replay
{
  namespace core = softadastra::wal::core;
  namespace reader = softadastra::wal::reader;

  /**
   * @brief WAL Replayer
   *
   * Responsible for replaying WAL records to rebuild state.
   */
  class WalReplayer
  {
  public:
    explicit WalReplayer(const std::string &path)
        : reader_(path)
    {
    }

    /**
     * @brief Replay all records from WAL
     *
     * @param apply Function applied to each record
     *
     * Guarantees:
     * - Sequential order
     * - Stops on corruption
     * - Deterministic replay
     */
    void replay(const std::function<void(const core::WalRecord &)> &apply)
    {
      reader_.for_each([&](const core::WalRecord &record)
                       {
        apply(record);
        last_sequence_ = record.sequence; });
    }

    /**
     * @brief Replay from a specific sequence (future optimization)
     */
    void replay_from(std::uint64_t start_seq,
                     const std::function<void(const core::WalRecord &)> &apply)
    {
      reader_.for_each([&](const core::WalRecord &record)
                       {
        if (record.sequence < start_seq)
          return;

        apply(record);
        last_sequence_ = record.sequence; });
    }

    /**
     * @brief Get last applied sequence
     */
    std::optional<std::uint64_t> last_sequence() const
    {
      return last_sequence_;
    }

  private:
    reader::WalReader reader_;
    std::optional<std::uint64_t> last_sequence_;
  };

} // namespace softadastra::wal::replay

#endif
