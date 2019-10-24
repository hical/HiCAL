cmake_minimum_required(VERSION 3.0)

target_include_directories(calengine
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include)

add_executable(tests nlp-utils.cc dataset.cc)
target_link_libraries(tests gtest_main calengine)

add_test(
    NAME all_test
    COMMAND ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/tests
    )
