# Changelog - softadastra/wal

All notable changes to the **wal module** are documented in this file.

The `wal` module provides **durable, ordered, append-only persistence** for all operations in Softadastra systems.
It is the foundation of **local-first guarantees, crash recovery, and deterministic replay**.

---

## [0.1.0] - Initial WAL Foundation

### Added

* Append-only log system
* `WalRecord` structure:

  * Sequence number
  * Operation type
  * Payload
  * Timestamp
* `WalWriter`:

  * Append operations
  * Durable writes (fsync or equivalent)
* `WalReader`:

  * Sequential read
  * Streaming iteration
* `Sequence` generator:

  * Monotonic increasing IDs
* Basic file-based WAL storage

### Guarantees

* Operations are persisted before being acknowledged
* Strict ordering of operations
* Replay produces deterministic results

### Design

* No dependency on higher-level modules
* Minimal and focused on durability

---

## [0.1.1] - Stability Improvements

### Improved

* Safer write handling to prevent partial record corruption
* More consistent sequence generation under concurrent writes
* Improved flush guarantees

### Fixed

* Edge cases with incomplete writes during abrupt shutdown
* Reader inconsistencies when reaching end-of-log

---

## [0.2.0] - Robustness Upgrade

### Added

* Record validation:

  * Basic integrity checks
* Safer read boundaries (detect truncated records)
* Error handling improvements for corrupted logs

### Improved

* WAL recovery behavior on restart
* Reader resilience to malformed entries

### Fixed

* Crash scenarios leading to invalid record parsing
* Incorrect handling of last partially written record

---

## [0.3.0] - Replay & Recovery Enhancements

### Added

* Replay API improvements:

  * Iteration helpers
  * Resume from sequence number
* Position tracking for recovery
* Basic checkpoint support (logical, not compacted)

### Improved

* Faster startup recovery
* More predictable replay ordering

---

## [0.4.0] - Storage Layer Improvements

### Added

* WAL store abstraction (`WalStore`)
* Preparation for:

  * Segmentation
  * Log rotation
* File management utilities

### Improved

* Separation between:

  * Write logic
  * Storage management

---

## [0.5.0] - Integrity & Safety

### Added

* Checksums for record validation (basic implementation)
* Detection of corrupted segments
* Safe truncation of invalid tail data

### Improved

* Crash safety during append
* Stronger guarantees on replay correctness

---

## [0.6.0] - Performance Improvements

### Added

* Buffered writes (optional)
* Batch append support

### Improved

* Write throughput under high load
* Reduced disk I/O overhead

---

## [0.7.0] - Extraction Ready

### Added

* Namespace stabilization (`softadastra::wal`)
* Public API cleanup
* Documentation for all exposed components

### Improved

* Decoupling from any application-specific assumptions
* Prepared for reuse in:

  * Softadastra Sync OS
  * SDK
  * Other local-first systems

---

## Roadmap

### Planned

* Log segmentation and rotation
* Compaction and pruning
* Streaming WAL replication
* Binary encoding optimization
* Cross-device WAL synchronization
* Snapshot + WAL hybrid model

---

## Philosophy

The WAL is the **source of truth**.

> If it is not in the WAL, it did not happen.

---

## Rules

* Always write before execution
* Never modify existing records
* Never break ordering
* Always recover deterministically

---

## Summary

The `wal` module guarantees:

* Durability
* Ordering
* Replayability
* Recovery

It is the **foundation of Softadastra’s reliability model**.
