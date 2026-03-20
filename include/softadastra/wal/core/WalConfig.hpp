/*
 * WalConfig.hpp
 */

#ifndef SOFTADASTRA_WAL_CONFIG_HPP
#define SOFTADASTRA_WAL_CONFIG_HPP

#include <string>
#include <cstddef>

namespace softadastra::wal::core
{
  struct WalConfig
  {
    /**
     * Path to WAL file
     */
    std::string path{"data/wal.log"};

    /**
     * Auto flush after each append
     */
    bool auto_flush{true};

    /**
     * Max WAL file size before rotation (future)
     */
    std::size_t max_file_size{64 * 1024 * 1024}; // 64 MB

    /**
     * Enable checksum (future)
     */
    bool enable_checksum{true};
  };

} // namespace softadastra::wal::core

#endif
