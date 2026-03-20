/*
 * test_writer.cpp
 */

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>

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

  const std::uint64_t seq1 = writer.append(record1);

  assert(seq1 == 1);
  assert(record1.sequence == 1);
  assert(record1.status == types::WalStatus::Persisted);

  core::WalRecord record2;
  record2.type = types::WalRecordType::Update;
  record2.timestamp = 123457;
  record2.payload = {5, 6};

  const std::uint64_t seq2 = writer.append(record2);

  assert(seq2 == 2);
  assert(record2.sequence == 2);
  assert(record2.status == types::WalStatus::Persisted);

  writer.flush();

  assert(std::filesystem::exists(wal_path));
  assert(std::filesystem::file_size(wal_path) > 0);

  std::filesystem::remove(wal_path);

  std::cout << "test_writer passed\n";
  return 0;
}
