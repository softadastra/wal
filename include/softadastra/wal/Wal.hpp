/**
 *
 *  @file Wal.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra WAL
 *
 */

#ifndef SOFTADASTRA_WAL_HPP
#define SOFTADASTRA_WAL_HPP

/**
 * @brief Main public aggregator header for the Softadastra WAL module.
 *
 * Include this file when you want access to the complete WAL public API.
 *
 * @code
 * #include <softadastra/wal/Wal.hpp>
 *
 * using namespace softadastra;
 *
 * int main()
 * {
 *   wal::writer::WalWriter writer{
 *       wal::core::WalConfig::durable("data/wal.log")};
 *
 *   auto result = writer.append(
 *       wal::types::WalRecordType::Put,
 *       wal::core::WalRecord::Payload{1, 2, 3});
 *
 *   return result.is_ok() ? 0 : 1;
 * }
 * @endcode
 */

/* Core */
#include <softadastra/wal/core/Sequence.hpp>
#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/core/WalRecord.hpp>

/* Types */
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/types/WalStatus.hpp>

/* Encoding */
#include <softadastra/wal/encoding/WalDecoder.hpp>
#include <softadastra/wal/encoding/WalEncoder.hpp>
#include <softadastra/wal/encoding/WalFormat.hpp>

/* Reader */
#include <softadastra/wal/reader/WalReader.hpp>

/* Replay */
#include <softadastra/wal/replay/WalReplayer.hpp>

/* Storage */
#include <softadastra/wal/storage/WalFile.hpp>
#include <softadastra/wal/storage/WalSegment.hpp>
#include <softadastra/wal/storage/WalStore.hpp>

/* Utils */
#include <softadastra/wal/utils/Checksum.hpp>
#include <softadastra/wal/utils/FileEventDeserializer.hpp>
#include <softadastra/wal/utils/FileEventSerializer.hpp>

/* Writer */
#include <softadastra/wal/writer/WalWriter.hpp>

#endif // SOFTADASTRA_WAL_HPP
