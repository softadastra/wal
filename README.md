# softadastra/wal

> Durable Write-Ahead Log for local-first systems.

The `wal` module is the **foundation of durability** in Softadastra.

It guarantees that:

> No accepted operation is ever lost, even in the presence of failures.

---

## Purpose

The goal of `softadastra/wal` is simple:

> Persist every operation before it is executed or synchronized.

This module ensures that the system can always:

* Recover after a crash
* Resume after disconnection
* Replay operations deterministically

---

## Core Principle

> Write first. Sync later.

Every operation must:

1. Be written to the WAL
2. Be flushed to durable storage
3. Then be processed or sent over the network

---

## Responsibilities

The `wal` module provides:

* Append-only log storage
* Durable persistence of operations
* Sequential ordering (monotonic sequence)
* Log replay capabilities
* Crash recovery

---

## What this module does NOT do

* No sync logic
* No network communication
* No filesystem watching
* No metadata management

👉 It only guarantees durability.

---

## Design Principles

### 1. Append-only

The WAL is never modified in place.

Only:

* Append
* Read
* Replay

---

### 2. Durable

An operation is considered valid only after it is persisted.

---

### 3. Ordered

All operations have a strict sequence:

* Monotonic increasing IDs
* Deterministic replay

---

### 4. Deterministic

Replaying the same WAL must produce the same state.

---

## Module Structure

```id="w4l9ax"
modules/wal/
├── include/softadastra/wal/
│   ├── WalRecord.hpp
│   ├── WalWriter.hpp
│   ├── WalReader.hpp
│   ├── WalStore.hpp
│   └── Sequence.hpp
└── src/
```

---

## Core Components

### WalRecord

Represents a single operation.

Typical fields:

* Sequence number
* Operation type
* Payload
* Timestamp

---

### WalWriter

Responsible for:

* Appending records
* Ensuring durability (fsync or equivalent)
* Managing write ordering

---

### WalReader

Provides:

* Sequential reading
* Streaming replay
* Iteration over records

---

### WalStore

Manages:

* WAL files
* Segmentation (future)
* Rotation (future)
* Storage lifecycle

---

### Sequence

Handles:

* Monotonic sequence generation
* Ordering guarantees
* Replay positioning

---

## Example Usage

```cpp id="ex3"
#include <softadastra/wal/WalWriter.hpp>
#include <softadastra/wal/WalRecord.hpp>

using namespace softadastra::wal;

WalWriter writer("data/wal.log");

WalRecord record;
record.type = "file_update";
record.payload = "...";

writer.append(record);
```

---

## Replay Example

```cpp id="ex4"
#include <softadastra/wal/WalReader.hpp>

using namespace softadastra::wal;

WalReader reader("data/wal.log");

reader.forEach([](const WalRecord& record) {
    // Apply operation
});
```

---

## Integration

Used by:

* Sync engine (primary)
* Metadata layer (indirectly)
* Application runtime

---

## Guarantees

The WAL ensures:

* No data loss after commit
* Ordered operations
* Replay after crash
* Consistent recovery

---

## Failure Model

The WAL is designed to survive:

* Process crash
* System crash
* Network failure
* Partial execution

---

## Dependencies

### Internal

* softadastra/core

### External

* Filesystem (POSIX / platform APIs)

---

## Roadmap

* Log segmentation
* Compaction
* Checksums and corruption detection
* Streaming WAL
* Replication-ready WAL
* Binary format optimization

---

## Rules

* Never modify existing records
* Never skip sequence numbers
* Never execute before persisting
* Always guarantee flush before ack

---

## Philosophy

The WAL is not just a log.

> It is the source of truth.

---

## Summary

* Guarantees durability
* Enables recovery
* Orders all operations
* Foundation of local-first systems

---

## License

See root LICENSE file.
