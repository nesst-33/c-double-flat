cmake_minimum_required(VERSION 3.23)
project(c_flat)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_library(LexerLib src/Lexer.cpp)
add_library(ParserLib src/Parser.cpp src/Node.cpp src/ASTPrinter.cpp)

add_executable(compiler src/main.cpp)
target_link_libraries(compiler PRIVATE LexerLib ParserLib)

cmake_policy(SET CMP0135 NEW)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(tests)
