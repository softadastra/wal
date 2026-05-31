/*
 * test_replay.cpp
 */

#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/replay/WalReplayer.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra::wal;

namespace
{
  [[nodiscard]] std::filesystem::path make_test_dir()
  {
    const auto unique_id =
        std::chrono::steady_clock::now()
            .time_since_epoch()
            .count();

    auto dir =
        std::filesystem::temp_directory_path() /
        ("softadastra_wal_replay_" + std::to_string(unique_id));

    std::filesystem::create_directories(dir);

    return dir;
  }
}

int main()
{
  const auto test_dir = make_test_dir();
  const auto wal_path = test_dir / "test_wal.log";

  core::WalConfig config;
  config.path = wal_path.string();
  config.auto_flush = true;

  writer::WalWriter writer(config);

  core::WalRecord record1(
      0,
      types::WalRecordType::Put,
      core::WalRecord::Payload{1, 2, 3, 4});

  auto seq1_result = writer.append(record1);
  assert(seq1_result.is_ok());
  assert(seq1_result.value() == 1);

  core::WalRecord record2(
      0,
      types::WalRecordType::Update,
      core::WalRecord::Payload{5, 6});

  auto seq2_result = writer.append(record2);
  assert(seq2_result.is_ok());
  assert(seq2_result.value() == 2);

  auto flush_result = writer.flush();
  assert(flush_result.is_ok());

  assert(std::filesystem::exists(wal_path));
  assert(std::filesystem::file_size(wal_path) > 0);

  replay::WalReplayer replayer(wal_path.string());

  std::vector<std::uint64_t> applied;

  auto replay_result = replayer.replay(
      [&](const core::WalRecord &record)
      {
        applied.push_back(record.sequence);
      });

  assert(replay_result.is_ok());

  assert(applied.size() == 2);
  assert(applied[0] == 1);
  assert(applied[1] == 2);

  auto last = replayer.last_sequence();
  assert(last.has_value());
  assert(*last == 2);

  std::error_code ec;
  std::filesystem::remove_all(test_dir, ec);

  std::cout << "test_replay passed\n";

  return 0;
}
