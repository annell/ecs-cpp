
find_package(GTest)

enable_testing()

add_executable(${PROJECT_NAME}_tests ECSTests.cpp)
target_link_libraries(${PROJECT_NAME}_tests GTest::gtest GTest::gtest_main ecs-cpp)
target_include_directories(${PROJECT_NAME}_tests PUBLIC ".")

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)