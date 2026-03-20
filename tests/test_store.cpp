/*
 * test_store.cpp
 */

#include <cassert>
#include <iostream>
#include <vector>
#include <cstdio>

#include <softadastra/wal/storage/WalFile.hpp>
#include <softadastra/wal/storage/WalSegment.hpp>
#include <softadastra/wal/storage/WalStore.hpp>
#include <softadastra/wal/core/WalConfig.hpp>

using namespace softadastra::wal;

static std::vector<std::uint8_t> make_data(std::uint8_t v, std::size_t n = 4)
{
  return std::vector<std::uint8_t>(n, v);
}

void test_wal_file_append_read()
{
  std::cout << "[test] wal_file_append_read\n";

  std::string path = "test_wal_file.log";

  {
    storage::WalFile file(path);

    auto d1 = make_data(1);
    auto d2 = make_data(2);

    file.append(d1);
    file.append(d2);
    file.flush();
  }

  {
    storage::WalFile file(path);

    auto data = file.read_all();

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

  std::string path = "test_wal_store.log";

  core::WalConfig config;
  config.path = path;
  config.auto_flush = true;

  storage::WalStore store(config);

  auto d1 = make_data(10);
  auto d2 = make_data(20);

  store.append(d1);
  store.append(d2);

  auto data = store.read_all();

  assert(data.size() == 8);
  assert(data[0] == 10);
  assert(data[4] == 20);

  std::remove(path.c_str());

  std::cout << "[ok] wal_store_basic\n";
}

void test_wal_segment_basic()
{
  std::cout << "[test] wal_segment_basic\n";

  std::string path = "test_segment.log";

  storage::WalSegment segment(
      1,    // id
      path, // path
      100   // start_sequence
  );

  assert(segment.id == 1);
  assert(segment.start_sequence == 100);
  assert(segment.end_sequence == 0);
  assert(segment.size == 0);

  // write through underlying file
  auto d = make_data(7);
  segment.file.append(d);
  segment.file.flush();

  auto data = segment.file.read_all();

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
