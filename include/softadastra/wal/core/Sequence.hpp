/*
 * Sequence.hpp
 */

#ifndef SOFTADASTRA_WAL_SEQUENCE_HPP
#define SOFTADASTRA_WAL_SEQUENCE_HPP

#include <cstdint>
#include <atomic>

namespace softadastra::wal::core
{
  class Sequence
  {
  public:
    Sequence() noexcept = default;

    explicit Sequence(std::uint64_t start) noexcept
        : value_(start)
    {
    }

    /**
     * @brief Get next sequence number (monotonic)
     */
    std::uint64_t next() noexcept
    {
      return value_.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * @brief Get current value (without increment)
     */
    std::uint64_t current() const noexcept
    {
      return value_.load(std::memory_order_relaxed);
    }

    /**
     * @brief Force set sequence (used for recovery)
     */
    void set(std::uint64_t value) noexcept
    {
      value_.store(value, std::memory_order_relaxed);
    }

  private:
    std::atomic<std::uint64_t> value_{0};
  };

} // namespace softadastra::wal::core

#endif
