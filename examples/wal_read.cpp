/*
 * wal_read.cpp
 */

#include <iostream>

#include <softadastra/wal/reader/WalReader.hpp>
#include <softadastra/wal/core/WalRecord.hpp>

using namespace softadastra;

int main()
{
  std::cout << "WAL read example\n";

  wal::reader::WalReader reader("wal.log");

  auto records = reader.read_all();

  for (const auto &r : records)
  {
    std::cout << "seq=" << r.sequence
              << " payload=" << (int)r.payload[0]
              << "\n";
  }

  return 0;
}
