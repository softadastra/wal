/**
 *
 *  @file Sequence.hpp
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

#ifndef SOFTADASTRA_WAL_SEQUENCE_HPP
#define SOFTADASTRA_WAL_SEQUENCE_HPP

#include <atomic>
#include <cstdint>

namespace softadastra::wal::core
{
  /**
   * @brief Thread-safe monotonic WAL sequence generator.
   *
   * Sequence generates increasing record numbers used to order WAL records.
   *
   * It is used by:
   * - WalWriter
   * - recovery logic
   * - replay ordering
   * - segment continuation
   *
   * Rules:
   * - The first generated value is 1 by default.
   * - Sequence values must only increase during normal writing.
   * - set() is intended for recovery or tests.
   * - Sequence does not persist itself.
   */
  class Sequence
  {
  public:
    /**
     * @brief Underlying sequence value type.
     */
    using value_type = std::uint64_t;

    /**
     * @brief Creates a sequence starting at 0.
     *
     * The first call to next() returns 1.
     */
    Sequence() noexcept = default;

    /**
     * @brief Creates a sequence with a custom current value.
     *
     * The next generated value will be start + 1.
     *
     * @param start Current sequence value.
     */
    explicit Sequence(value_type start) noexcept
        : value_(start)
    {
    }

    Sequence(const Sequence &) = delete;
    Sequence &operator=(const Sequence &) = delete;

    /**
     * @brief Move-constructs a sequence.
     *
     * The moved value is copied atomically from the source.
     */
    Sequence(Sequence &&other) noexcept
        : value_(other.current())
    {
    }

    /**
     * @brief Move-assigns a sequence.
     *
     * The moved value is copied atomically from the source.
     */
    Sequence &operator=(Sequence &&other) noexcept
    {
      if (this != &other)
      {
        set(other.current());
      }

      return *this;
    }

    /**
     * @brief Returns the next monotonic sequence number.
     *
     * @return Next sequence number.
     */
    [[nodiscard]] value_type next() noexcept
    {
      return value_.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    /**
     * @brief Returns the current sequence value without incrementing it.
     *
     * @return Current sequence value.
     */
    [[nodiscard]] value_type current() const noexcept
    {
      return value_.load(std::memory_order_relaxed);
    }

    /**
     * @brief Sets the current sequence value.
     *
     * This is mainly used after WAL recovery, when the writer must continue
     * from the highest sequence already found on disk.
     *
     * @param value New current sequence value.
     */
    void set(value_type value) noexcept
    {
      value_.store(value, std::memory_order_relaxed);
    }

    /**
     * @brief Resets the sequence to zero.
     *
     * After reset(), the next call to next() returns 1.
     */
    void reset() noexcept
    {
      set(0);
    }

    /**
     * @brief Returns true if no sequence has been generated yet.
     *
     * @return true when current value is zero.
     */
    [[nodiscard]] bool empty() const noexcept
    {
      return current() == 0;
    }

  private:
    std::atomic<value_type> value_{0};
  };

} // namespace softadastra::wal::core

#endif // SOFTADASTRA_WAL_SEQUENCE_HPP
