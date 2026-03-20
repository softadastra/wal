/*
 * WalFile.hpp
 */

#ifndef SOFTADASTRA_WAL_FILE_HPP
#define SOFTADASTRA_WAL_FILE_HPP

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

namespace softadastra::wal::storage
{
  class WalFile
  {
  public:
    explicit WalFile(const std::string &path)
        : path_(path)
    {
      open();
    }

    ~WalFile()
    {
      close();
    }

    /**
     * @brief Append bytes to WAL (no flush)
     */
    void append(const std::vector<std::uint8_t> &data)
    {
      stream_.write(reinterpret_cast<const char *>(data.data()),
                    static_cast<std::streamsize>(data.size()));
    }

    /**
     * @brief Force flush to disk
     */
    void flush()
    {
      stream_.flush();
    }

    /**
     * @brief Read entire WAL file
     */
    std::vector<std::uint8_t> read_all()
    {
      std::ifstream in(path_, std::ios::binary);

      if (!in)
        return {};

      in.seekg(0, std::ios::end);
      std::size_t size = static_cast<std::size_t>(in.tellg());
      in.seekg(0, std::ios::beg);

      std::vector<std::uint8_t> buffer(size);
      in.read(reinterpret_cast<char *>(buffer.data()),
              static_cast<std::streamsize>(size));

      return buffer;
    }

    /**
     * @brief Check if file is open
     */
    bool is_open() const
    {
      return stream_.is_open();
    }

  private:
    void open()
    {
      stream_.open(path_,
                   std::ios::binary |
                       std::ios::out |
                       std::ios::app);

      if (!stream_)
      {
        throw std::runtime_error("Failed to open WAL file: " + path_);
      }
    }

    void close()
    {
      if (stream_.is_open())
        stream_.close();
    }

  private:
    std::string path_;
    std::ofstream stream_;
  };

} // namespace softadastra::wal::storage

#endif
