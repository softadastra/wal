/*
 * wal_stream.cpp
 */

#include <iostream>

#include <softadastra/wal/reader/WalReader.hpp>
#include <softadastra/wal/core/WalRecord.hpp>

using namespace softadastra;

int main()
{
  std::cout << "WAL streaming example\n";

  wal::reader::WalReader reader("wal.log");

  reader.for_each([](const wal::core::WalRecord &r)
                  { std::cout << "stream seq=" << r.sequence
                              << " size=" << r.payload.size()
                              << "\n"; });

  return 0;
}
