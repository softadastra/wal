/*
 * WalSegment.hpp
 */

#ifndef SOFTADASTRA_WAL_SEGMENT_HPP
#define SOFTADASTRA_WAL_SEGMENT_HPP

#include <string>
#include <cstdint>

#include <softadastra/wal/storage/WalFile.hpp>

namespace softadastra::wal::storage
{
  struct WalSegment
  {
    /**
     * Segment ID (monotonic)
     */
    std::uint64_t id{0};

    /**
     * Path to segment file
     */
    std::string path;

    /**
     * First sequence stored in this segment
     */
    std::uint64_t start_sequence{0};

    /**
     * Last sequence stored
     */
    std::uint64_t end_sequence{0};

    /**
     * Current size in bytes
     */
    std::uint64_t size{0};

    /**
     * Underlying WAL file
     */
    WalFile file;

    WalSegment(std::uint64_t id_,
               const std::string &path_,
               std::uint64_t start_seq)
        : id(id_),
          path(path_),
          start_sequence(start_seq),
          file(path_)
    {
    }
  };

} // namespace softadastra::wal::storage

#endif
