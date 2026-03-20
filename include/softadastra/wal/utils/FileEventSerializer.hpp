/*
 * FileEventSerializer.hpp
 */

#ifndef SOFTADASTRA_WAL_FILE_EVENT_SERIALIZER_HPP
#define SOFTADASTRA_WAL_FILE_EVENT_SERIALIZER_HPP

#include <vector>
#include <string>

#include <softadastra/fs/events/FileEvent.hpp>

namespace softadastra::wal::utils
{
  namespace fs_events = softadastra::fs::events;
  namespace types = softadastra::fs::types;

  class FileEventSerializer
  {
  public:
    static std::vector<std::uint8_t> serialize(const fs_events::FileEvent &ev)
    {
      std::vector<std::uint8_t> out;

      // type
      out.push_back(static_cast<std::uint8_t>(ev.type));

      // path
      const std::string &path = ev.current.path.str();
      std::uint32_t len = path.size();

      append(out, len);
      out.insert(out.end(), path.begin(), path.end());

      return out;
    }

  private:
    template <typename T>
    static void append(std::vector<std::uint8_t> &buf, T v)
    {
      auto *p = reinterpret_cast<std::uint8_t *>(&v);
      buf.insert(buf.end(), p, p + sizeof(T));
    }
  };

} // namespace softadastra::wal::utils

#endif
