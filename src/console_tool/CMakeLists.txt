file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_executable (console_tool ${SRCS})

target_link_libraries (console_tool emu6502)

target_include_directories (console_tool PRIVATE ${CMAKE_SOURCE_DIR}/src/emu6502)