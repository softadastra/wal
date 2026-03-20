/*
 * wal_write.cpp
 */

#include <iostream>

#include <softadastra/wal/writer/WalWriter.hpp>
#include <softadastra/wal/core/WalRecord.hpp>

using namespace softadastra;

int main()
{
  std::cout << "WAL write example\n";

  wal::core::WalConfig config;
  config.path = "wal.log";
  config.auto_flush = true;

  wal::writer::WalWriter writer(config);

  for (int i = 1; i <= 5; ++i)
  {
    wal::core::WalRecord r;
    r.timestamp = i;
    r.payload = {static_cast<uint8_t>(i)};

    auto seq = writer.append(r);

    std::cout << "written seq=" << seq << "\n";
  }

  return 0;
}
