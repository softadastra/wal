# WAL Guide

The Softadastra WAL module provides a durable Write-Ahead Log for local-first systems.

It is used to persist operations before they are applied, synchronized, or replayed.

The core rule is simple:

> *Write first. Apply later.*

## Why Softadastra needs a WAL

Local-first systems must survive unstable conditions:

- process crashes
- system restarts
- network failures
- partial execution
- offline periods
- interrupted synchronization

Without a WAL, an operation can be accepted by the application but lost before it reaches storage or the network. The WAL prevents this.

When an operation is appended successfully, it receives a durable sequence number and can be replayed later.

## What the WAL guarantees

- accepted operations are written before being applied
- records are ordered by monotonic sequence numbers
- records can be read sequentially
- records can be replayed deterministically
- corrupted or incomplete trailing records are not applied

> The WAL does not guarantee synchronization by itself.
> Synchronization is handled by higher-level modules.

## What the WAL does NOT do

- network communication
- conflict resolution
- filesystem watching
- metadata indexing
- distributed consensus
- application state management

> It only stores ordered durable records.
> Higher-level modules decide what each record means.

## Installation

```bash
vix add @softadastra/wal
```

### Main header

```cpp
#include <softadastra/wal/Wal.hpp>
```

This gives access to the full public WAL API:

- `softadastra::wal::core`
- `softadastra::wal::types`
- `softadastra::wal::encoding`
- `softadastra::wal::reader`
- `softadastra::wal::replay`
- `softadastra::wal::storage`
- `softadastra::wal::utils`
- `softadastra::wal::writer`

## Basic write example

```cpp
#include <softadastra/wal/Wal.hpp>

using namespace softadastra;

int main()
{
    wal::writer::WalWriter writer{
        wal::core::WalConfig::durable("data/wal.log")};

    wal::core::WalRecord::Payload payload{1, 2, 3, 4};

    auto result = writer.append(
        wal::types::WalRecordType::Put,
        std::move(payload));

    if (result.is_err())
    {
        return 1;
    }

    auto sequence = result.value();

    return sequence > 0 ? 0 : 1;
}
```

## Configuration

Production:

```cpp
auto config = wal::core::WalConfig::durable("data/wal.log");
```

This enables:

- automatic flush
- checksum verification
- default maximum file size of 64 MiB

Tests or benchmarks:

```cpp
auto config = wal::core::WalConfig::fast("data/wal.log");
```

The fast configuration disables automatic flush to reduce write overhead.

## WAL records

A WAL record is represented by `WalRecord`. It contains:

- `sequence`
- `type`
- `status`
- `timestamp`
- `payload`

```cpp
wal::core::WalRecord record{
    0,
    wal::types::WalRecordType::Put,
    wal::core::WalRecord::Payload{10, 20, 30}};
```

The sequence is assigned by `WalWriter`. The payload is opaque binary data — the WAL does not interpret it.

## Record types

`WalRecordType` describes the logical meaning of a record:

- `wal::types::WalRecordType::Put`
- `wal::types::WalRecordType::Update`
- `wal::types::WalRecordType::Delete`
- `wal::types::WalRecordType::Checkpoint`
- `wal::types::WalRecordType::Noop`

Use `to_string()` for logs:

```cpp
auto text = wal::types::to_string(record.type);
```

## Record status

`WalStatus` describes the lifecycle of a record:

- `wal::types::WalStatus::Pending`
- `wal::types::WalStatus::Persisted`
- `wal::types::WalStatus::Applied`
- `wal::types::WalStatus::Failed`

A record is usually created as `Pending`, then marked as `Persisted` by the writer.

## Reading records

```cpp
#include <softadastra/wal/Wal.hpp>

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
        // inspect record
    }

    return 0;
}
```

For large WAL files, prefer streaming:

```cpp
wal::reader::WalReader reader{"data/wal.log"};

auto result = reader.for_each(
    [](const wal::core::WalRecord &record)
    {
        // process record
    });
```

## Replaying records

```cpp
#include <softadastra/wal/Wal.hpp>

using namespace softadastra;

int main()
{
    wal::replay::WalReplayer replayer{"data/wal.log"};

    auto result = replayer.replay(
        [](const wal::core::WalRecord &record)
        {
            // apply record deterministically
        });

    if (result.is_err())
    {
        return 1;
    }

    return 0;
}
```

Replay from a specific sequence:

```cpp
auto result = replayer.replay_from(
    42,
    [](const wal::core::WalRecord &record)
    {
        // apply records with sequence >= 42
    });
```

## Filesystem event persistence

The WAL can persist filesystem events from `softadastra/fs`:

```cpp
#include <softadastra/wal/Wal.hpp>
#include <softadastra/fs/events/FileEvent.hpp>

using namespace softadastra;

int main()
{
    wal::writer::WalWriter writer{
        wal::core::WalConfig::durable("data/fs.log")};

    fs::events::FileEvent event;

    auto result = writer.append_event(event);

    return result.is_ok() ? 0 : 1;
}
```

Filesystem events are mapped to WAL record types:

| Event | WAL Record Type |
|-------|----------------|
| `Created` | `Put` |
| `Updated` | `Update` |
| `Deleted` | `Delete` |

The event payload is encoded by `FileEventSerializer`.

## Binary format

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

All integer values are encoded in **little-endian** order.
The checksum is computed over the payload only.

## Corruption handling

When reading records, the WAL stops safely at the first invalid record. A record can be rejected if:

- the magic value is invalid
- the version is unsupported
- the payload size is too large
- the record is incomplete
- the checksum does not match
- the decoded record is structurally invalid

This allows the system to recover all valid records before a corrupted or partial trailing write.

## Error handling

The WAL public API uses `softadastra::core::types::Result`:

```cpp
auto result = writer.flush();

if (result.is_err())
{
    const auto &error = result.error();
}
```

Public operations return errors instead of throwing for normal failures.

## Sequence numbers

`Sequence` generates monotonic record numbers:

```cpp
wal::core::Sequence sequence;

auto first  = sequence.next();  // 1
auto second = sequence.next();  // 2
```

After recovery, a writer can continue from the highest known sequence:

```cpp
writer.set_sequence(last_sequence);
```

## Recommended write flow

A correct local-first flow:

1. Build operation payload
2. Append operation to WAL
3. Ensure append succeeded
4. Apply operation locally
5. Sync operation later

```cpp
auto result = writer.append(
    wal::types::WalRecordType::Put,
    std::move(payload));

if (result.is_err())
{
    return;
}

// safe to apply after WAL acceptance
apply_operation();
```

## Design rules

- write before apply
- never mutate existing records in place
- keep sequence order stable
- keep payload interpretation outside the WAL core
- replay records deterministically
- treat the WAL as the recovery source

## Common usage patterns

### Persist a generic operation

```cpp
writer.append(
    wal::types::WalRecordType::Put,
    std::move(payload));
```

### Persist a filesystem event

```cpp
writer.append_event(event);
```

### Read every record

```cpp
reader.for_each(
    [](const wal::core::WalRecord &record)
    {
        // process record
    });
```

### Replay into a store

```cpp
replayer.replay(
    [&](const wal::core::WalRecord &record)
    {
        store.apply(record);
    });
```

## Production notes

Use `WalConfig::durable()` for production paths:

```cpp
auto config = wal::core::WalConfig::durable("data/wal.log");
```

For high-throughput workloads, batching and segment rotation can be added above the current API without changing the user-facing writer flow.

## Summary

`softadastra/wal` provides:

- append-only persistence
- monotonic ordering
- stable encoding
- checksum verification
- deterministic reading
- deterministic replay

> Its job is simple: make accepted operations recoverable.
