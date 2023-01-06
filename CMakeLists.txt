cmake_minimum_required(VERSION 3.20.1)
project(ECSCppTests)

set(CMAKE_CXX_STANDARD 20)


find_package(GTest)

enable_testing()

add_executable(${PROJECT_NAME} tests/ECSTests.cpp)
target_link_libraries(${PROJECT_NAME} GTest::gtest GTest::gtest_main)
target_include_directories(${PROJECT_NAME} PUBLIC ".")

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})