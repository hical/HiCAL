# CALEngine

A framework to build Continuous Active Learning processes.

# Requirements

* cmake
* g++/clang++
* libfcgi
* libarchive
* gtest
* spawn-fcgi (to run `bmi_fcgi`)

# Installation

```bash
$ mkdir build && cd build && cmake .. && cmake --build .

# Run the tests
build/ $ ctest --verbose

# Binaries
build/ $ ls bin/
```
