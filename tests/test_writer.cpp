/*
 * test_writer.cpp
 */

#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>

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
        ("softadastra_wal_writer_" + std::to_string(unique_id));

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

  const std::uint64_t seq1 = seq1_result.value();

  assert(seq1 == 1);
  assert(record1.sequence == 1);
  assert(record1.type == types::WalRecordType::Put);
  assert(record1.status == types::WalStatus::Persisted);
  assert(record1.timestamp.is_valid());
  assert(record1.payload.size() == 4);

  core::WalRecord record2(
      0,
      types::WalRecordType::Update,
      core::WalRecord::Payload{5, 6});

  auto seq2_result = writer.append(record2);
  assert(seq2_result.is_ok());

  const std::uint64_t seq2 = seq2_result.value();

  assert(seq2 == 2);
  assert(record2.sequence == 2);
  assert(record2.type == types::WalRecordType::Update);
  assert(record2.status == types::WalStatus::Persisted);
  assert(record2.timestamp.is_valid());
  assert(record2.payload.size() == 2);

  auto flushed = writer.flush();
  assert(flushed.is_ok());

  assert(writer.current_sequence() == 2);
  assert(std::filesystem::exists(wal_path));
  assert(std::filesystem::file_size(wal_path) > 0);

  std::error_code ec;
  std::filesystem::remove_all(test_dir, ec);

  std::cout << "test_writer passed\n";

  return 0;
}
