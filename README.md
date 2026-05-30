# softadastra/wal

> Durable Write-Ahead Log primitives for reliable Softadastra products.

`softadastra/wal` provides the durability layer of the Softadastra C++ stack.

Softadastra builds reliability-first products for local-first, offline-first, and distributed applications. This module persists operations before they are applied, synchronized, or replayed.

## Purpose

`softadastra/wal` exists to make accepted operations durable.

It is used by higher-level modules such as Store, Sync, Metadata, SDKs, and product infrastructure.

The core rule is simple:

> Write first. Apply later.

It is designed to be:

- Durable
- Ordered
- Replayable
- Deterministic
- Product-ready

## What it provides

This module provides WAL primitives such as:

- Append-only records
- Monotonic sequence numbers
- Stable binary encoding
- Payload checksums
- Durable writes
- Sequential reading
- Deterministic replay
- Recovery after restart or crash

## What it does not do

`softadastra/wal` does not contain:

- Sync logic
- Network transport
- Conflict resolution
- Filesystem watching
- Metadata indexing
- Application state management

It stores operations and preserves order. Higher-level modules decide what those operations mean.

## Core model

```txt
Operation
    |
Append to WAL
    |
Durable record
    |
Replay / Apply / Sync
```

Once an operation is successfully appended to the WAL, it has a durable sequence number and can be replayed later.

## Where it fits

```txt
Softadastra products
        |
SDKs and product APIs
        |
Sync, Store, Metadata
        |
softadastra/wal
        |
softadastra/fs
        |
softadastra/core
```

`softadastra/wal` depends on the lower-level foundation and provides durability for higher-level modules.

## Installation

```bash
vix add @softadastra/wal
```

## Usage

```cpp
#include <softadastra/wal/Wal.hpp>
```

## Example

```cpp
#include <softadastra/wal/Wal.hpp>

#include <iostream>
#include <vector>

int main()
{
    auto config = softadastra::wal::core::WalConfig::durable("data/wal.log");

    softadastra::wal::writer::WalWriter writer{config};

    softadastra::wal::core::WalRecord::Payload payload{
        'h', 'e', 'l', 'l', 'o'
    };

    auto result = writer.append(
        softadastra::wal::types::WalRecordType::Put,
        std::move(payload)
    );

    if (result.is_err())
    {
        std::cout << result.error().message() << "\n";
        return 1;
    }

    std::cout << "Appended record #" << result.value() << "\n";
    return 0;
}
```

## Requirements

- C++20
- `softadastra/core`
- Filesystem support for durable WAL files

## Documentation

For the full documentation, visit [docs.softadastra.com](https://docs.softadastra.com).

## License

Licensed under the Apache License, Version 2.0.
