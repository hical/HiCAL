cmake_minimum_required(VERSION 3.15)

add_library(calengine STATIC
    ./utils/text-utils.cc
    ./utils/sf-sparse-vector.cc
    ./dataset/dataset-memory.cc
    ./featurizer/tfidf.cc
    )

target_include_directories(calengine
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include)

add_executable(main main.cc)
target_link_libraries(main calengine)

add_executable(corpus_parser corpus_parser.cc)
target_link_libraries(corpus_parser calengine archive)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
