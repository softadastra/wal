/*
 * WalReader.hpp
 */

#ifndef SOFTADASTRA_WAL_READER_HPP
#define SOFTADASTRA_WAL_READER_HPP

#include <fstream>
#include <vector>
#include <functional>
#include <optional>
#include <cstdint>

#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/encoding/WalDecoder.hpp>
#include <softadastra/wal/encoding/WalFormat.hpp>

namespace softadastra::wal::reader
{
  namespace core = softadastra::wal::core;
  namespace encoding = softadastra::wal::encoding;

  class WalReader
  {
  public:
    explicit WalReader(const std::string &path)
        : path_(path)
    {
    }

    /**
     * @brief Stream records sequentially
     *
     * Stops safely if corruption is detected
     */
    void for_each(const std::function<void(const core::WalRecord &)> &callback)
    {
      std::ifstream in(path_, std::ios::binary);

      if (!in)
        return;

      while (true)
      {
        auto record = read_next(in);

        if (!record.has_value())
          break;

        callback(*record);
      }
    }

    /**
     * @brief Read all records (non-streaming)
     */
    std::vector<core::WalRecord> read_all()
    {
      std::vector<core::WalRecord> records;

      for_each([&](const core::WalRecord &r)
               { records.push_back(r); });

      return records;
    }

  private:
    std::optional<core::WalRecord> read_next(std::ifstream &in)
    {
      // --- read header first ---
      std::vector<std::uint8_t> header;
      header.resize(encoding::WalFormat::HEADER_SIZE);

      in.read(reinterpret_cast<char *>(header.data()),
              encoding::WalFormat::HEADER_SIZE);

      if (in.gcount() != static_cast<std::streamsize>(encoding::WalFormat::HEADER_SIZE))
        return std::nullopt;

      // extract payload size (last 4 bytes of header)
      std::uint32_t payload_size = 0;
      std::memcpy(&payload_size,
                  header.data() + (encoding::WalFormat::HEADER_SIZE - sizeof(std::uint32_t)),
                  sizeof(std::uint32_t));

      // total record size
      const std::size_t total_size =
          encoding::WalFormat::HEADER_SIZE +
          payload_size +
          encoding::WalFormat::CHECKSUM_SIZE;

      std::vector<std::uint8_t> buffer;
      buffer.resize(total_size);

      // copy header
      std::memcpy(buffer.data(), header.data(), header.size());

      // read rest (payload + checksum)
      in.read(reinterpret_cast<char *>(buffer.data() + header.size()),
              static_cast<std::streamsize>(total_size - header.size()));

      if (in.gcount() != static_cast<std::streamsize>(total_size - header.size()))
        return std::nullopt;

      // decode
      return encoding::WalDecoder::decode(buffer.data(), buffer.size());
    }

  private:
    std::string path_;
  };

} // namespace softadastra::wal::reader

#endif
