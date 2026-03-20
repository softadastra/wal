/*
 * WalEncoder.hpp
 */

#ifndef SOFTADASTRA_WAL_ENCODER_HPP
#define SOFTADASTRA_WAL_ENCODER_HPP

#include <vector>
#include <cstdint>
#include <cstring>

#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/encoding/WalFormat.hpp>

namespace softadastra::wal::encoding
{
  namespace core = softadastra::wal::core;

  class WalEncoder
  {
  public:
    static std::vector<std::uint8_t> encode(const core::WalRecord &record)
    {
      const std::uint32_t payload_size =
          static_cast<std::uint32_t>(record.payload.size());

      const std::size_t total_size =
          WalFormat::HEADER_SIZE +
          payload_size +
          WalFormat::CHECKSUM_SIZE;

      std::vector<std::uint8_t> buffer;
      buffer.resize(total_size);

      std::size_t offset = 0;

      // sequence
      write(buffer, offset, record.sequence);

      // type
      write(buffer, offset, static_cast<std::uint8_t>(record.type));

      // status
      write(buffer, offset, static_cast<std::uint8_t>(record.status));

      // timestamp
      write(buffer, offset, record.timestamp);

      // payload size
      write(buffer, offset, payload_size);

      // payload
      if (payload_size > 0)
      {
        std::memcpy(buffer.data() + offset,
                    record.payload.data(),
                    payload_size);
        offset += payload_size;
      }

      // checksum
      const std::uint32_t checksum = compute_checksum(buffer.data(), offset);
      write(buffer, offset, checksum);

      return buffer;
    }

  private:
    template <typename T>
    static void write(std::vector<std::uint8_t> &buffer,
                      std::size_t &offset,
                      T value)
    {
      std::memcpy(buffer.data() + offset, &value, sizeof(T));
      offset += sizeof(T);
    }

    static std::uint32_t compute_checksum(const std::uint8_t *data,
                                          std::size_t size)
    {
      // simple FNV-1a (fast, no dependency)
      std::uint32_t hash = 2166136261u;

      for (std::size_t i = 0; i < size; ++i)
      {
        hash ^= data[i];
        hash *= 16777619u;
      }

      return hash;
    }
  };

} // namespace softadastra::wal::encoding

#endif
