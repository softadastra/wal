/*
 * test_record.cpp
 */

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/types/WalStatus.hpp>

using namespace softadastra::wal::core;
using namespace softadastra::wal::types;

void test_empty_record()
{
  std::cout << "[test] empty_record\n";

  WalRecord r;

  assert(r.sequence == 0);
  assert(r.type == WalRecordType::Unknown);
  assert(r.status == WalStatus::Pending);
  assert(!r.has_sequence());
  assert(!r.has_payload());
  assert(r.payload_size() == 0);
  assert(!r.is_valid());

  std::cout << "[ok] empty_record\n";
}

void test_record_basic()
{
  std::cout << "[test] record_basic\n";

  WalRecord r(
      42,
      WalRecordType::Put,
      WalRecord::Payload{1, 2, 3});

  assert(r.sequence == 42);
  assert(r.type == WalRecordType::Put);
  assert(r.status == WalStatus::Pending);
  assert(r.has_sequence());
  assert(r.has_payload());
  assert(r.payload_size() == 3);
  assert(r.payload[0] == 1);
  assert(r.payload[1] == 2);
  assert(r.payload[2] == 3);
  assert(r.timestamp.is_valid());
  assert(r.is_valid());

  std::cout << "[ok] record_basic\n";
}

void test_record_make()
{
  std::cout << "[test] record_make\n";

  WalRecord r = WalRecord::make(
      7,
      WalRecordType::Update,
      WalRecord::Payload{10, 20});

  assert(r.sequence == 7);
  assert(r.type == WalRecordType::Update);
  assert(r.status == WalStatus::Pending);
  assert(r.payload_size() == 2);
  assert(r.timestamp.is_valid());
  assert(r.is_valid());

  std::cout << "[ok] record_make\n";
}

void test_record_status_transitions()
{
  std::cout << "[test] record_status_transitions\n";

  WalRecord r(
      1,
      WalRecordType::Put,
      WalRecord::Payload{42});

  assert(r.status == WalStatus::Pending);

  r.mark_persisted();
  assert(r.status == WalStatus::Persisted);

  r.mark_applied();
  assert(r.status == WalStatus::Applied);

  r.mark_failed();
  assert(r.status == WalStatus::Failed);

  std::cout << "[ok] record_status_transitions\n";
}

void test_record_clear()
{
  std::cout << "[test] record_clear\n";

  WalRecord r(
      99,
      WalRecordType::Delete,
      WalRecord::Payload{1});

  assert(r.is_valid());

  r.clear();

  assert(r.sequence == 0);
  assert(r.type == WalRecordType::Unknown);
  assert(r.status == WalStatus::Pending);
  assert(!r.has_sequence());
  assert(!r.has_payload());
  assert(r.payload_size() == 0);
  assert(!r.timestamp.is_valid());
  assert(!r.is_valid());

  std::cout << "[ok] record_clear\n";
}

int main()
{
  test_empty_record();
  test_record_basic();
  test_record_make();
  test_record_status_transitions();
  test_record_clear();

  return 0;
}
