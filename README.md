# softadastra/wal

> Durable Write-Ahead Log for local-first systems.

`softadastra/wal` is the durability layer of Softadastra.

It persists operations before they are executed, synchronized, or replayed.

The core rule is simple:

> *Write first. Apply later.*

## Purpose

The WAL exists to guarantee that accepted operations are not lost after a crash, restart, or network failure.

It allows Softadastra to:

- persist operations durably
- replay operations deterministically
- recover after process or system failure
- resume work after disconnection
- preserve operation ordering

## Core Guarantee

Once an operation is successfully appended to the WAL, it has a durable sequence number and can be replayed later.

```cpp
auto result = writer.append(type, payload);
```

If `result.is_ok()` returns `true`, the operation was accepted by the WAL.

## What this module does

`softadastra/wal` provides:

- append-only WAL records
- monotonic sequence numbers
- stable binary encoding
- payload checksums
- sequential reading
- deterministic replay
- basic filesystem event persistence helpers

## What this module does NOT do

- synchronization
- networking
- conflict resolution
- filesystem watching
- metadata indexing
- application state management

> The WAL stores bytes and preserves order.
> Higher-level modules decide what those bytes mean.

## Design Principles

### Append-only

Records are appended to the log. Existing records are never modified in place.

### Durable

A record is considered accepted only after the storage append succeeds.

### Ordered

Every record receives a monotonic sequence number. Replay follows WAL order.

### Deterministic

Reading the same WAL in the same order must produce the same replay stream.

### Storage-agnostic payload

A payload can represent:

- a filesystem event
- a sync operation
- a metadata mutation
- a checkpoint
- an application command

## Module Structure

```
include/softadastra/wal/
├── core/
│   ├── Sequence.hpp
│   ├── WalConfig.hpp
│   └── WalRecord.hpp
├── encoding/
│   ├── WalDecoder.hpp
│   ├── WalEncoder.hpp
│   └── WalFormat.hpp
├── reader/
│   └── WalReader.hpp
├── replay/
│   └── WalReplayer.hpp
├── storage/
│   ├── WalFile.hpp
│   ├── WalSegment.hpp
│   └── WalStore.hpp
├── types/
│   ├── WalRecordType.hpp
│   └── WalStatus.hpp
├── utils/
│   ├── Checksum.hpp
│   ├── FileEventDeserializer.hpp
│   └── FileEventSerializer.hpp
└── writer/
    └── WalWriter.hpp
```

## Core Types

### `WalRecord`

A single operation stored in the WAL. It contains:

- `sequence`
- `type`
- `status`
- `timestamp`
- `payload` (opaque binary data)

```cpp
wal::core::WalRecord record{
    0,
    wal::types::WalRecordType::Put,
    payload};
```

### `WalWriter`

High-level append API. Assigns the sequence number, encodes the record, and appends it to storage.

```cpp
wal::writer::WalWriter writer{
    wal::core::WalConfig::durable("data/wal.log")};

auto result = writer.append(
    wal::types::WalRecordType::Put,
    payload);

if (result.is_ok())
{
    auto sequence = result.value();
}
```

### `WalReader`

Sequential reader for WAL files.

```cpp
wal::reader::WalReader reader{"data/wal.log"};

auto result = reader.for_each(
    [](const wal::core::WalRecord &record)
    {
        // inspect or process record
    });

if (result.is_err())
{
    // handle read error
}
```

### `WalReplayer`

Deterministic replay helper.

```cpp
wal::replay::WalReplayer replayer{"data/wal.log"};

auto result = replayer.replay(
    [](const wal::core::WalRecord &record)
    {
        // apply record deterministically
    });

if (result.is_ok() && replayer.has_replayed())
{
    auto last = replayer.last_sequence();
}
```

### `Sequence`

Thread-safe monotonic sequence generator.

```cpp
wal::core::Sequence sequence;

auto first  = sequence.next();  // 1
auto second = sequence.next();  // 2
```

## Filesystem Event Example

`wal` can persist events produced by `softadastra/fs`:

```cpp
#include <softadastra/wal/writer/WalWriter.hpp>
#include <softadastra/fs/events/FileEvent.hpp>

using namespace softadastra;

int main()
{
    wal::writer::WalWriter writer{
        wal::core::WalConfig::durable("data/wal.log")};

    fs::events::FileEvent event;

    auto result = writer.append_event(event);

    if (result.is_err())
    {
        return 1;
    }

    return 0;
}
```

`append_event()` maps filesystem events to WAL record types:

| Event | WAL Record Type |
|-------|----------------|
| `Created` | `Put` |
| `Updated` | `Update` |
| `Deleted` | `Delete` |

## Generic Payload Example

Use this when the caller already has serialized bytes:

```cpp
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra;

int main()
{
    wal::writer::WalWriter writer{
        wal::core::WalConfig::durable("data/wal.log")};

    wal::core::WalRecord::Payload payload{
        'h', 'e', 'l', 'l', 'o'};

    auto result = writer.append(
        wal::types::WalRecordType::Put,
        std::move(payload));

    return result.is_ok() ? 0 : 1;
}
```

## Read All Records

```cpp
#include <softadastra/wal/reader/WalReader.hpp>

using namespace softadastra;

int main()
{
    wal::reader::WalReader reader{"data/wal.log"};

    auto records = reader.read_all();

    if (records.is_err())
    {
        return 1;
    }

    for (const auto &record : records.value())
    {
        // process record
    }

    return 0;
}
```

## Binary Format

Current format version: `1`

```
uint32  magic
uint32  version
uint64  sequence
uint8   record_type
uint8   status
int64   timestamp_millis
uint32  payload_size
bytes   payload
uint32  checksum
```

Integer values are encoded in **little-endian** order.
The checksum is computed over the payload only.

## Configuration

```cpp
auto config = wal::core::WalConfig::durable("data/wal.log");
```

Default durable behavior:

- auto flush enabled
- checksum enabled
- max WAL file size set to 64 MiB

For tests or benchmarks:

```cpp
auto config = wal::core::WalConfig::fast("data/wal.log");
```

## Error Handling

The WAL API uses `softadastra::core::types::Result`. Public operations do not throw for normal failures.

```cpp
auto result = writer.flush();

if (result.is_err())
{
    const auto &error = result.error();
}
```

## Failure Model

The WAL is designed for systems that may face:

- process crashes
- restarts
- network disconnection
- partial execution
- interrupted replay
- corrupted trailing records

When reading, the WAL stops safely at the first invalid, incomplete, or corrupted record.

## Rules

- Write before apply
- Never mutate existing records in place
- Preserve sequence order
- Treat the WAL as the recovery source
- Keep payload interpretation outside the WAL core

## Installation

```bash
vix add @softadastra/wal
```

## License

See the root `LICENSE` file.
