cmake_minimum_required(VERSION 3.16)

find_package(GTest REQUIRED)

add_executable(csvparser_tests
    test_main.cpp
    test_csvparser.cpp
)

target_link_libraries(csvparser_tests
    PRIVATE
        csvparser
        GTest::GTest
        GTest::Main
)

target_compile_features(csvparser_tests PRIVATE cxx_std_17)

add_test(
    NAME csvparser_tests
    COMMAND csvparser_tests
)
