cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER /opt/homebrew/Cellar/llvm/20.1.6/bin/clang)
set(CMAKE_CXX_COMPILER /opt/homebrew/Cellar/llvm/20.1.6/bin/clang++)

project(luz_tests)

enable_testing()

find_package(Catch2 REQUIRED)

# TODO how to link without explicity naming buffer.cc?
add_executable(protocol_test protocol_test.cc ${CMAKE_CURRENT_SOURCE_DIR}/../protocol.cc ${CMAKE_CURRENT_SOURCE_DIR}/../buffer.cc)
target_compile_options(protocol_test PRIVATE -std=c++23)

target_include_directories(protocol_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_directories(protocol_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(protocol_test PRIVATE Catch2::Catch2WithMain)

target_link_libraries(protocol_test PRIVATE /opt/homebrew/Cellar/llvm/20.1.6/lib/c++/libc++.a)
target_link_libraries(protocol_test PRIVATE /opt/homebrew/Cellar/llvm/20.1.6/lib/c++/libc++abi.a)

include(CTest)
include(Catch)

catch_discover_tests(protocol_test)
