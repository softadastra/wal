/*
 * test_reader.cpp
 */

#include <cassert>
#include <iostream>

#include <softadastra/wal/reader/WalReader.hpp>

using namespace softadastra::wal;

int main()
{
  reader::WalReader reader("test_wal.log");

  auto records = reader.read_all();

  assert(!records.empty());

  assert(records[0].sequence == 0);
  assert(records[0].payload.size() == 4);

  assert(records[1].sequence == 1);
  assert(records[1].payload.size() == 2);

  std::cout << "test_reader passed\n";
}
