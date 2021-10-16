include_directories(../3rd_parties/doctest/doctest)

include_directories(../src/emu6502)

add_executable (emu_test test.cpp)
target_link_libraries (emu_test PRIVATE emu6502)
add_test (NAME emu_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/emu_test)
