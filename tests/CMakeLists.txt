enable_testing()

add_executable(
    lexer_tests
    lexer_tests.cpp 
)

target_link_libraries(
    lexer_tests
    GTest::gtest_main
    LexerLib
)

add_executable(
    parser_tests
    parser_tests.cpp
)

target_link_libraries(
    parser_tests
    GTest::gtest_main
    LexerLib
    ParserLib
)

add_executable(
    value_tests
    value_tests.cpp
)

target_link_libraries(
    value_tests
    GTest::gtest_main
    InterpreterLib
)


include(GoogleTest)
gtest_discover_tests(lexer_tests)
gtest_discover_tests(parser_tests)
gtest_discover_tests(value_tests)
