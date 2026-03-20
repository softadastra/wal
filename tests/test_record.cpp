/*
 * test_record.cpp
 */

#include <cassert>
#include <iostream>

#include <softadastra/wal/core/WalRecord.hpp>

using namespace softadastra::wal::core;

void test_record_basic()
{
  std::cout << "[test] record_basic\n";

  WalRecord r;
  r.sequence = 42;
  r.timestamp = 123456;

  r.payload = {1, 2, 3};

  assert(r.sequence == 42);
  assert(r.payload.size() == 3);

  std::cout << "[ok] record_basic\n";
}

int main()
{
  test_record_basic();
  return 0;
}
