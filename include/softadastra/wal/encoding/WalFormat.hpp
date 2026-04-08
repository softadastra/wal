/*
 * WalFormat.hpp
 */

#ifndef SOFTADASTRA_WAL_FORMAT_HPP
#define SOFTADASTRA_WAL_FORMAT_HPP

#include <cstddef>
#include <cstdint>

namespace softadastra::wal::encoding
{
  /**
   * WAL binary format (v1)
   *
   * Layout:
   *
   * | sequence (8 bytes) |
   * | type     (1 byte)  |
   * | status   (1 byte)  |
   * | timestamp (8 bytes)|
   * | payload_size (4)   |
   * | payload (N bytes)  |
   * | checksum (4 bytes) |
   *
   */

  struct WalFormat
  {
    static constexpr std::uint32_t MAGIC = 0x57414C31; // "WAL1"
    static constexpr std::uint32_t VERSION = 1;

    static constexpr std::size_t HEADER_SIZE =
        sizeof(std::uint64_t) + // sequence
        sizeof(std::uint8_t) +  // type
        sizeof(std::uint8_t) +  // status
        sizeof(std::uint64_t) + // timestamp
        sizeof(std::uint32_t);  // payload size

    static constexpr std::size_t CHECKSUM_SIZE =
        sizeof(std::uint32_t);
  };

} // namespace softadastra::wal::encoding

#endif
