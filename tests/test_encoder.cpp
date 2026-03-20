/*
 * test_encoder.cpp
 */

#include <cassert>
#include <iostream>
#include <optional>

#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/state/FileMetadata.hpp>
#include <softadastra/fs/state/FileState.hpp>
#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/types/FileEventType.hpp>
#include <softadastra/fs/types/FileType.hpp>

#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/types/WalStatus.hpp>
#include <softadastra/wal/encoding/WalEncoder.hpp>
#include <softadastra/wal/encoding/WalDecoder.hpp>
#include <softadastra/wal/utils/FileEventSerializer.hpp>
#include <softadastra/wal/utils/FileEventDeserializer.hpp>

using namespace softadastra;

void test_encode_decode_file_event()
{
  std::cout << "[test] encode_decode_file_event\n";

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

  wal::core::WalRecord record;
  record.sequence = 1;
  record.type = wal::types::WalRecordType::Put;
  record.status = wal::types::WalStatus::Pending;
  record.timestamp = 999;
  record.payload = wal::utils::FileEventSerializer::serialize(event);

  auto buffer = wal::encoding::WalEncoder::encode(record);
  auto decoded = wal::encoding::WalDecoder::decode(buffer.data(), buffer.size());

  assert(decoded.has_value());

  const auto &decoded_record = *decoded;

  assert(decoded_record.sequence == record.sequence);
  assert(decoded_record.type == record.type);
  assert(decoded_record.status == record.status);
  assert(decoded_record.timestamp == record.timestamp);
  assert(decoded_record.payload == record.payload);

  auto decoded_event = wal::utils::FileEventDeserializer::deserialize(decoded_record.payload);
  assert(decoded_event.has_value());

  assert(decoded_event->type == fs::types::FileEventType::Created);
  assert(decoded_event->current.path.str() == "docs/file.txt");
  assert(decoded_event->current.metadata.type == fs::types::FileType::File);
  assert(!decoded_event->previous.has_value());

  std::cout << "[ok] encode_decode_file_event\n";
}

int main()
{
  test_encode_decode_file_event();
  return 0;
}
