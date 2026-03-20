cmake_minimum_required(VERSION 3.20)

# ========================
# Tests
# ========================

add_executable(test_sequence test_sequence.cpp)
target_link_libraries(test_sequence PRIVATE softadastra_wal)
add_test(NAME wal_sequence COMMAND test_sequence)

add_executable(test_record test_record.cpp)
target_link_libraries(test_record PRIVATE softadastra_wal)
add_test(NAME wal_record COMMAND test_record)

add_executable(test_encoder test_encoder.cpp)
target_link_libraries(test_encoder PRIVATE softadastra_wal)
add_test(NAME wal_encoder COMMAND test_encoder)

add_executable(test_writer test_writer.cpp)
target_link_libraries(test_writer PRIVATE softadastra_wal)
add_test(NAME wal_writer COMMAND test_writer)

add_executable(test_reader test_reader.cpp)
target_link_libraries(test_reader PRIVATE softadastra_wal)
add_test(NAME wal_reader COMMAND test_reader)

add_executable(test_replay test_replay.cpp)
target_link_libraries(test_replay PRIVATE softadastra_wal)
add_test(NAME wal_replay COMMAND test_replay)

add_executable(test_store test_store.cpp)
target_link_libraries(test_store PRIVATE softadastra_wal)
add_test(NAME wal_store COMMAND test_store)

add_executable(test_append_event test_append_event.cpp)
target_link_libraries(test_append_event PRIVATE softadastra_wal)
add_test(NAME wal_append_event COMMAND test_append_event)
