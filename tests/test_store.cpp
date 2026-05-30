/*
 * test_store.cpp
 */

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/storage/WalFile.hpp>
#include <softadastra/wal/storage/WalSegment.hpp>
#include <softadastra/wal/storage/WalStore.hpp>

using namespace softadastra::wal;

static std::vector<std::uint8_t> make_data(std::uint8_t v, std::size_t n = 4)
{
  return std::vector<std::uint8_t>(n, v);
}

void test_wal_file_append_read()
{
  std::cout << "[test] wal_file_append_read\n";

  const std::string path = "test_wal_file.log";

  {
    storage::WalFile file(path);

    auto d1 = make_data(1);
    auto d2 = make_data(2);

    auto append1 = file.append(d1);
    assert(append1.is_ok());

    auto append2 = file.append(d2);
    assert(append2.is_ok());

    auto flushed = file.flush();
    assert(flushed.is_ok());
  }

  {
    storage::WalFile file(path);

    auto result = file.read_all();
    assert(result.is_ok());

    const auto &data = result.value();

    assert(!data.empty());
    assert(data.size() == 8); // 4 + 4
    assert(data[0] == 1);
    assert(data[4] == 2);
  }

  std::remove(path.c_str());

  std::cout << "[ok] wal_file_append_read\n";
}

void test_wal_store_basic()
{
  std::cout << "[test] wal_store_basic\n";

  const std::string path = "test_wal_store.log";

  core::WalConfig config;
  config.path = path;
  config.auto_flush = true;

  storage::WalStore store(config);

  auto d1 = make_data(10);
  auto d2 = make_data(20);

  auto append1 = store.append(d1);
  assert(append1.is_ok());

  auto append2 = store.append(d2);
  assert(append2.is_ok());

  auto result = store.read_all();
  assert(result.is_ok());

  const auto &data = result.value();

  assert(data.size() == 8);
  assert(data[0] == 10);
  assert(data[4] == 20);
  assert(store.bytes_written() == 8);

  std::remove(path.c_str());

  std::cout << "[ok] wal_store_basic\n";
}

void test_wal_segment_basic()
{
  std::cout << "[test] wal_segment_basic\n";

  const std::string path = "test_segment.log";

  storage::WalSegment segment(
      1,    // id
      path, // path
      100   // start_sequence
  );

  assert(segment.id() == 1);
  assert(segment.start_sequence() == 100);
  assert(segment.end_sequence() == 0);
  assert(segment.size() == 0);
  assert(segment.empty());

  auto d = make_data(7);

  auto appended = segment.append(100, d);
  assert(appended.is_ok());

  auto flushed = segment.flush();
  assert(flushed.is_ok());

  assert(segment.id() == 1);
  assert(segment.start_sequence() == 100);
  assert(segment.end_sequence() == 100);
  assert(segment.size() == 4);
  assert(!segment.empty());

  auto result = segment.file().read_all();
  assert(result.is_ok());

  const auto &data = result.value();

  assert(data.size() == 4);
  assert(data[0] == 7);

  std::remove(path.c_str());

  std::cout << "[ok] wal_segment_basic\n";
}

int main()
{
  test_wal_file_append_read();
  test_wal_store_basic();
  test_wal_segment_basic();

  std::cout << "\nAll WAL storage tests passed.\n";

  return 0;
}
