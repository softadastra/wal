/*
 * test_sequence.cpp
 */

#include <cassert>
#include <iostream>

#include <softadastra/wal/core/Sequence.hpp>

using namespace softadastra::wal::core;

void test_sequence_increment()
{
  std::cout << "[test] sequence_increment\n";

  Sequence seq;

  auto a = seq.next();
  auto b = seq.next();
  auto c = seq.next();

  assert(a == 1);
  assert(b == 2);
  assert(c == 3);

  std::cout << "[ok] sequence_increment\n";
}

int main()
{
  test_sequence_increment();
  return 0;
}
