cmake_minimum_required(VERSION 3.15)
project(calengine)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-W -Wall -Werror)

if(NOT CMAKE_BUILD_TYPE)
    message("Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                                         "MinSizeRel" "RelWithDebInfo")
endif()

find_program(CCACHE ccache)
if(CCACHE)
    message("using ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
    message("ccache not found")
endif()

# Generate compile_commands.json to make it easier to work with clang based
# tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Main targets
add_subdirectory(src)

# Testing
add_subdirectory(lib/gtest-1.10)
enable_testing()
add_subdirectory(test)
