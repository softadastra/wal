/*
 * test_writer.cpp
 */

#include <cassert>
#include <iostream>

#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra::wal;

int main()
{
  core::WalConfig config;
  config.path = "test_wal.log";
  config.auto_flush = true;

  writer::WalWriter writer(config);

  core::WalRecord record;
  record.type = types::WalRecordType::Put;
  record.timestamp = 123456;

  record.payload = {1, 2, 3, 4};

  std::uint64_t seq1 = writer.append(record);

  assert(seq1 == 0);
  assert(record.status == types::WalStatus::Persisted);

  core::WalRecord record2;
  record2.type = types::WalRecordType::Update;
  record2.timestamp = 123457;
  record2.payload = {5, 6};

  std::uint64_t seq2 = writer.append(record2);

  assert(seq2 == 1);

  std::cout << "test_writer passed\n";
}
