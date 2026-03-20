/*
 * test_replay.cpp
 */

#include <cassert>
#include <iostream>
#include <vector>

#include <softadastra/wal/replay/WalReplayer.hpp>

using namespace softadastra::wal;

int main()
{
  replay::WalReplayer replayer("test_wal.log");

  std::vector<std::uint64_t> applied;

  replayer.replay([&](const core::WalRecord &record)
                  { applied.push_back(record.sequence); });

  assert(applied.size() >= 2);
  assert(applied[0] == 0);
  assert(applied[1] == 1);

  auto last = replayer.last_sequence();
  assert(last.has_value());
  assert(*last == 1);

  std::cout << "test_replay passed\n";
}
