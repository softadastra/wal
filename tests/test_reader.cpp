/*
 * test_reader.cpp
 */

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <vector>

#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/state/FileMetadata.hpp>
#include <softadastra/fs/state/FileState.hpp>
#include <softadastra/fs/types/FileEventType.hpp>
#include <softadastra/fs/types/FileType.hpp>

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/reader/WalReader.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>
#include <softadastra/wal/utils/FileEventDeserializer.hpp>

using namespace softadastra;

void test_reader_reads_written_event()
{
  std::cout << "[test] reader_reads_written_event\n";

  const std::string wal_path = "test_reader_wal.log";
  std::filesystem::remove(wal_path);

  wal::core::WalConfig config;
  config.path = wal_path;
  config.auto_flush = true;

  wal::writer::WalWriter writer(config);

  auto path_result = fs::path::Path::from("docs/file.txt");
  assert(path_result.is_ok());

  fs::state::FileMetadata metadata;
  metadata.type = fs::types::FileType::File;

  fs::state::FileState current{
      path_result.value(),
      metadata,
      std::nullopt};

  fs::events::FileEvent event{
      fs::types::FileEventType::Created,
      current,
      std::nullopt};

  const std::uint64_t seq = writer.append_event(event);
  assert(seq == 1);

  writer.flush();

  assert(std::filesystem::exists(wal_path));
  assert(std::filesystem::file_size(wal_path) > 0);

  wal::reader::WalReader reader(config.path);

  auto records = reader.read_all();
  assert(records.size() == 1);

  const auto &record = records[0];

  assert(record.sequence == 1);
  assert(record.type == wal::types::WalRecordType::Put);
  assert(record.timestamp > 0);
  assert(!record.payload.empty());

  auto decoded_event = wal::utils::FileEventDeserializer::deserialize(record.payload);
  assert(decoded_event.has_value());

  assert(decoded_event->type == fs::types::FileEventType::Created);
  assert(decoded_event->current.path.str() == "docs/file.txt");
  assert(decoded_event->current.metadata.type == fs::types::FileType::File);
  assert(!decoded_event->previous.has_value());

  std::filesystem::remove(wal_path);

  std::cout << "[ok] reader_reads_written_event\n";
}

int main()
{
  test_reader_reads_written_event();
  return 0;
}
