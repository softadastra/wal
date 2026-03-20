/*
 * WalRecord.hpp
 */

#ifndef SOFTADASTRA_WAL_RECORD_HPP
#define SOFTADASTRA_WAL_RECORD_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/types/WalStatus.hpp>

namespace softadastra::wal::core
{
  namespace types = softadastra::wal::types;

  struct WalRecord
  {
    std::uint64_t sequence{0};

    types::WalRecordType type{types::WalRecordType::Unknown};
    types::WalStatus status{types::WalStatus::Pending};

    std::uint64_t timestamp{0};

    std::vector<std::uint8_t> payload;
  };

} // namespace softadastra::wal::core

#endif
