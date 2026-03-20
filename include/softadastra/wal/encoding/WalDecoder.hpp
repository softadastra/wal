/*
 * WalDecoder.hpp
 */

#ifndef SOFTADASTRA_WAL_DECODER_HPP
#define SOFTADASTRA_WAL_DECODER_HPP

#include <cstdint>
#include <cstring>
#include <vector>
#include <optional>

#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/encoding/WalFormat.hpp>

namespace softadastra::wal::encoding
{
  namespace core = softadastra::wal::core;
  namespace types = softadastra::wal::types;

  class WalDecoder
  {
  public:
    static std::optional<core::WalRecord> decode(const std::uint8_t *data,
                                                 std::size_t size)
    {
      if (size < WalFormat::HEADER_SIZE + WalFormat::CHECKSUM_SIZE)
        return std::nullopt;

      std::size_t offset = 0;

      core::WalRecord record;

      // sequence
      read(data, offset, record.sequence);

      // type
      std::uint8_t type;
      read(data, offset, type);
      record.type = static_cast<types::WalRecordType>(type);

      // status
      std::uint8_t status;
      read(data, offset, status);
      record.status = static_cast<types::WalStatus>(status);

      // timestamp
      read(data, offset, record.timestamp);

      // payload size
      std::uint32_t payload_size;
      read(data, offset, payload_size);

      if (offset + payload_size + WalFormat::CHECKSUM_SIZE > size)
        return std::nullopt;

      // payload
      if (payload_size > 0)
      {
        record.payload.resize(payload_size);
        std::memcpy(record.payload.data(),
                    data + offset,
                    payload_size);
        offset += payload_size;
      }

      // checksum
      std::uint32_t stored_checksum;
      read(data, offset, stored_checksum);

      const std::uint32_t computed =
          compute_checksum(data, offset - sizeof(std::uint32_t));

      if (stored_checksum != computed)
        return std::nullopt;

      return record;
    }

  private:
    template <typename T>
    static void read(const std::uint8_t *data,
                     std::size_t &offset,
                     T &value)
    {
      std::memcpy(&value, data + offset, sizeof(T));
      offset += sizeof(T);
    }

    static std::uint32_t compute_checksum(const std::uint8_t *data,
                                          std::size_t size)
    {
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
