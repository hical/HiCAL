cmake_minimum_required(VERSION 3.15)

include(GoogleTest)
include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${gmock_SOURCE_DIR}/include)
target_include_directories(calengine
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include)

add_executable(tests nlp-utils.cc dataset.cc)
target_link_libraries(tests gtest_main calengine)

gtest_discover_tests(tests)
