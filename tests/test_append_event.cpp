/*
 * test_append_event.cpp
 */

#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/state/FileMetadata.hpp>
#include <softadastra/fs/state/FileState.hpp>
#include <softadastra/fs/types/FileEventType.hpp>
#include <softadastra/fs/types/FileType.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra;

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
        ("softadastra_wal_append_event_" + std::to_string(unique_id));

    std::filesystem::create_directories(dir);

    return dir;
  }
}

int main()
{
  const auto test_dir = make_test_dir();
  const auto wal_path = test_dir / "test_wal_event.log";

  wal::core::WalConfig config;
  config.path = wal_path.string();
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

  auto seq_result = writer.append_event(event);
  assert(seq_result.is_ok());

  const std::uint64_t seq = seq_result.value();
  assert(seq == 1);

  auto flush_result = writer.flush();
  assert(flush_result.is_ok());

  assert(writer.current_sequence() == 1);
  assert(std::filesystem::exists(wal_path));
  assert(std::filesystem::file_size(wal_path) > 0);

  std::error_code ec;
  std::filesystem::remove_all(test_dir, ec);

  std::cout << "test_append_event passed\n";

  return 0;
}
