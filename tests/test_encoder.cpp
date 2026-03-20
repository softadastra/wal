/*
 * test_encoder.cpp
 */

#include <cassert>
#include <iostream>

#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/encoding/WalEncoder.hpp>
#include <softadastra/wal/encoding/WalDecoder.hpp>

using namespace softadastra::wal::core;
using namespace softadastra::wal::encoding;

void test_encode_decode()
{
  std::cout << "[test] encode_decode\n";

  WalRecord r;
  r.sequence = 1;
  r.timestamp = 999;
  r.payload = {10, 20, 30, 40};

  auto buffer = WalEncoder::encode(r);

  auto decoded = WalDecoder::decode(buffer.data(), buffer.size());

  assert(decoded.has_value());

  auto d = *decoded;

  assert(d.sequence == r.sequence);
  assert(d.timestamp == r.timestamp);
  assert(d.payload == r.payload);

  std::cout << "[ok] encode_decode\n";
}

int main()
{
  test_encode_decode();
  return 0;
}
