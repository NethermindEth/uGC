cmake_minimum_required(VERSION 3.20)

project(ugc)

option(UGC_BUILD_TESTS "Build uGC core unit tests" OFF)

# The ugc-zero library needs the .NET runtime headers; skip it when they are
# not provided (e.g. host-only unit-test builds).
if(DEFINED UGC_RUNTIME_INCLUDE_DIR)
    add_subdirectory(ugc)
else()
    message(STATUS "UGC_RUNTIME_INCLUDE_DIR not set - skipping ugc-zero library")
endif()

if(UGC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
