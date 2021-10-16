file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_executable(console_tool ${SRCS})
target_include_directories(console_tool PRIVATE ../emu6502)

target_link_libraries(console_tool emu6502)
