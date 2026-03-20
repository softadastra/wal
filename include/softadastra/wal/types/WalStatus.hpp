/*
 * WalStatus.hpp
 */

#ifndef SOFTADASTRA_WAL_STATUS_HPP
#define SOFTADASTRA_WAL_STATUS_HPP

#include <cstdint>

namespace softadastra::wal::types
{

  enum class WalStatus : std::uint8_t
  {
    Pending = 0,
    Persisted,
    Applied,
    Failed
  };

} // namespace softadastra::wal::types

#endif
