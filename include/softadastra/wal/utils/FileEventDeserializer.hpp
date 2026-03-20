/*
 * FileEventDeserializer.hpp
 */

#ifndef SOFTADASTRA_WAL_FILE_EVENT_DESERIALIZER_HPP
#define SOFTADASTRA_WAL_FILE_EVENT_DESERIALIZER_HPP

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/state/FileMetadata.hpp>
#include <softadastra/fs/state/FileState.hpp>
#include <softadastra/fs/types/FileEventType.hpp>
#include <softadastra/fs/types/FileType.hpp>

namespace softadastra::wal::utils
{
  namespace fs_events = softadastra::fs::events;
  namespace fs_path = softadastra::fs::path;
  namespace fs_state = softadastra::fs::state;
  namespace fs_types = softadastra::fs::types;

  class FileEventDeserializer
  {
  public:
    static std::optional<fs_events::FileEvent> deserialize(const std::vector<std::uint8_t> &data)
    {
      if (data.empty())
      {
        return std::nullopt;
      }

      return deserialize(data.data(), data.size());
    }

    static std::optional<fs_events::FileEvent> deserialize(const std::uint8_t *data,
                                                           std::size_t size)
    {
      if (data == nullptr)
      {
        return std::nullopt;
      }

      if (size < sizeof(std::uint8_t) + sizeof(std::uint32_t))
      {
        return std::nullopt;
      }

      std::size_t offset = 0;

      std::uint8_t raw_type = 0;
      read(data, offset, raw_type);

      std::uint32_t path_len = 0;
      read(data, offset, path_len);

      if (offset + path_len > size)
      {
        return std::nullopt;
      }

      std::string path_str(reinterpret_cast<const char *>(data + offset), path_len);
      offset += path_len;

      auto path_result = fs_path::Path::from(path_str);
      if (path_result.is_err())
      {
        return std::nullopt;
      }

      fs_state::FileMetadata metadata;
      metadata.type = fs_types::FileType::File;

      fs_state::FileState current{
          path_result.value(),
          metadata,
          std::nullopt};

      fs_events::FileEvent event{
          static_cast<fs_types::FileEventType>(raw_type),
          current,
          std::nullopt};

      return event;
    }

  private:
    template <typename T>
    static void read(const std::uint8_t *data,
                     std::size_t &offset,
                     T &value)
    {
      std::memcpy(&value, data + offset, sizeof(T));
      offset += sizeof(T);
    }
  };

} // namespace softadastra::wal::utils

#endif
