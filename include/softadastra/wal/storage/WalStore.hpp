/*
 * WalStore.hpp
 */

#ifndef SOFTADASTRA_WAL_STORE_HPP
#define SOFTADASTRA_WAL_STORE_HPP

#include <memory>
#include <vector>

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/storage/WalFile.hpp>

namespace softadastra::wal::storage
{
  namespace core = softadastra::wal::core;

  class WalStore
  {
  public:
    explicit WalStore(const core::WalConfig &config)
        : config_(config),
          file_(std::make_unique<WalFile>(config.path))
    {
    }

    /**
     * @brief Append record bytes to WAL
     */
    void append(const std::vector<std::uint8_t> &data)
    {
      file_->append(data);

      if (config_.auto_flush)
      {
        file_->flush();
      }
    }

    /**
     * @brief Force flush
     */
    void flush()
    {
      file_->flush();
    }

    /**
     * @brief Read full WAL
     */
    std::vector<std::uint8_t> read_all()
    {
      return file_->read_all();
    }

  private:
    core::WalConfig config_;
    std::unique_ptr<WalFile> file_;
  };

} // namespace softadastra::wal::storage

#endif
