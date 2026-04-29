/**
 *
 *  @file WalSegment.hpp
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

#ifndef SOFTADASTRA_WAL_SEGMENT_HPP
#define SOFTADASTRA_WAL_SEGMENT_HPP

#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/core/Core.hpp>
#include <softadastra/wal/storage/WalFile.hpp>

namespace softadastra::wal::storage
{
  namespace core_types = softadastra::core::types;
  namespace core_errors = softadastra::core::errors;

  /**
   * @brief Metadata and file handle for one WAL segment.
   *
   * WalSegment represents a single append-only WAL file.
   *
   * A segment tracks:
   * - its monotonic segment id
   * - its file path
   * - the first sequence stored in the segment
   * - the last sequence stored in the segment
   * - the current encoded byte size
   * - the underlying WalFile
   *
   * WalSegment does not encode records.
   * WalStore or WalWriter are responsible for producing encoded bytes.
   */
  class WalSegment : public core_types::NonCopyable
  {
  public:
    /**
     * @brief Result type for operations without return value.
     */
    using Result = core_types::Result<void, core_errors::Error>;

    /**
     * @brief Creates an empty segment.
     */
    WalSegment() = default;

    /**
     * @brief Creates a WAL segment.
     *
     * The underlying file is not opened automatically.
     *
     * @param segment_id Monotonic segment id.
     * @param segment_path Segment file path.
     * @param start_sequence First sequence expected in this segment.
     */
    WalSegment(
        std::uint64_t segment_id,
        std::string segment_path,
        std::uint64_t start_sequence)
        : id_(segment_id),
          path_(std::move(segment_path)),
          start_sequence_(start_sequence),
          file_(path_)
    {
    }

    WalSegment(WalSegment &&) noexcept = default;
    WalSegment &operator=(WalSegment &&) noexcept = default;

    /**
     * @brief Opens the underlying WAL segment file.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result open()
    {
      return file_.open();
    }

    /**
     * @brief Appends encoded bytes to the segment.
     *
     * The sequence range and size are updated only after the append succeeds.
     *
     * @param sequence Sequence number of the encoded record.
     * @param bytes Encoded WAL record bytes.
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result append(
        std::uint64_t sequence,
        const std::vector<std::uint8_t> &bytes)
    {
      auto result = file_.append(bytes);

      if (result.is_err())
      {
        return result;
      }

      if (start_sequence_ == 0)
      {
        start_sequence_ = sequence;
      }

      end_sequence_ = sequence;
      size_ += static_cast<std::uint64_t>(bytes.size());

      return Result::ok();
    }

    /**
     * @brief Flushes the underlying segment file.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] Result flush()
    {
      return file_.flush();
    }

    /**
     * @brief Closes the underlying segment file.
     */
    void close() noexcept
    {
      file_.close();
    }

    /**
     * @brief Returns true if the segment file is open.
     *
     * @return true if open.
     */
    [[nodiscard]] bool is_open() const noexcept
    {
      return file_.is_open();
    }

    /**
     * @brief Returns true if the segment contains no records.
     *
     * @return true when no end sequence is known.
     */
    [[nodiscard]] bool empty() const noexcept
    {
      return end_sequence_ == 0;
    }

    /**
     * @brief Returns true if appending bytes would exceed a max size.
     *
     * @param next_size Number of bytes about to be appended.
     * @param max_size Maximum allowed segment size.
     * @return true if the segment should rotate before appending.
     */
    [[nodiscard]] bool would_exceed(
        std::uint64_t next_size,
        std::uint64_t max_size) const noexcept
    {
      return max_size > 0 && size_ + next_size > max_size;
    }

    /**
     * @brief Returns the segment id.
     *
     * @return Segment id.
     */
    [[nodiscard]] std::uint64_t id() const noexcept
    {
      return id_;
    }

    /**
     * @brief Returns the segment path.
     *
     * @return Segment file path.
     */
    [[nodiscard]] const std::string &path() const noexcept
    {
      return path_;
    }

    /**
     * @brief Returns the first sequence stored in this segment.
     *
     * @return Start sequence.
     */
    [[nodiscard]] std::uint64_t start_sequence() const noexcept
    {
      return start_sequence_;
    }

    /**
     * @brief Returns the last sequence stored in this segment.
     *
     * @return End sequence.
     */
    [[nodiscard]] std::uint64_t end_sequence() const noexcept
    {
      return end_sequence_;
    }

    /**
     * @brief Returns the current segment size in bytes.
     *
     * @return Segment size in bytes.
     */
    [[nodiscard]] std::uint64_t size() const noexcept
    {
      return size_;
    }

    /**
     * @brief Returns the underlying WAL file.
     *
     * @return WAL file reference.
     */
    [[nodiscard]] WalFile &file() noexcept
    {
      return file_;
    }

    /**
     * @brief Returns the underlying WAL file.
     *
     * @return WAL file const reference.
     */
    [[nodiscard]] const WalFile &file() const noexcept
    {
      return file_;
    }

  private:
    std::uint64_t id_{0};
    std::string path_{};
    std::uint64_t start_sequence_{0};
    std::uint64_t end_sequence_{0};
    std::uint64_t size_{0};
    WalFile file_{};
  };

} // namespace softadastra::wal::storage

#endif // SOFTADASTRA_WAL_SEGMENT_HPP
