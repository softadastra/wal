/*
 * test_replay.cpp
 */

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <vector>

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/replay/WalReplayer.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra::wal;

int main()
{
  const std::string wal_path = "test_wal.log";
  std::filesystem::remove(wal_path);

  core::WalConfig config;
  config.path = wal_path;
  config.auto_flush = true;

  writer::WalWriter writer(config);

  core::WalRecord record1;
  record1.type = types::WalRecordType::Put;
  record1.timestamp = 123456;
  record1.payload = {1, 2, 3, 4};
  writer.append(record1);

  core::WalRecord record2;
  record2.type = types::WalRecordType::Update;
  record2.timestamp = 123457;
  record2.payload = {5, 6};
  writer.append(record2);

  writer.flush();

  replay::WalReplayer replayer(wal_path);

  std::vector<std::uint64_t> applied;

  replayer.replay([&](const core::WalRecord &record)
                  { applied.push_back(record.sequence); });

  assert(applied.size() == 2);
  assert(applied[0] == 1);
  assert(applied[1] == 2);

  auto last = replayer.last_sequence();
  assert(last.has_value());
  assert(*last == 2);

  std::filesystem::remove(wal_path);

  std::cout << "test_replay passed\n";
  return 0;
}
