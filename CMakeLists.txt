cmake_minimum_required (VERSION 3.1)
project (emu6502)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory (src)
add_subdirectory (3rd_parties)

enable_testing ()
add_subdirectory (tests)