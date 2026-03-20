cmake_minimum_required(VERSION 3.20)

# ========================
# Examples
# ========================

add_executable(wal_write wal_write.cpp)
target_link_libraries(wal_write PRIVATE softadastra_wal)

add_executable(wal_read wal_read.cpp)
target_link_libraries(wal_read PRIVATE softadastra_wal)

add_executable(wal_stream wal_stream.cpp)
target_link_libraries(wal_stream PRIVATE softadastra_wal)
