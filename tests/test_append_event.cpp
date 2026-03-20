/*
 * test_append_event.cpp
 */

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>

#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/state/FileState.hpp>
#include <softadastra/fs/state/FileMetadata.hpp>
#include <softadastra/fs/types/FileEventType.hpp>
#include <softadastra/fs/types/FileType.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra;

int main()
{
  const std::string wal_path = "test_wal_event.log";

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

  std::filesystem::remove(wal_path);

  std::cout << "test_append_event passed\n";
  return 0;
}
