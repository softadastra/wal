/*
 * WalRecordType.hpp
 */

#ifndef SOFTADASTRA_WAL_RECORD_TYPE_HPP
#define SOFTADASTRA_WAL_RECORD_TYPE_HPP

#include <cstdint>

namespace softadastra::wal::types
{

  enum class WalRecordType : std::uint8_t
  {
    Unknown = 0,

    // Generic operations
    Put,
    Update,
    Delete,

    // System
    Checkpoint,
    Noop
  };

} // namespace softadastra::wal::types

#endif
