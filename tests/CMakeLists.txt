# uGC unit tests
#
# Copyright (C) 2026 Demerzel Solutions Limited (Nethermind)
# Author: Maxim Menshikov <maksim.menshikov@nethermind.io>

add_library(ugc-core-host
            STATIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../ugc/core/ugc_core.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../ugc/core/ugc_zalloc.c)
target_include_directories(ugc-core-host
                           PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/../ugc/core)

foreach(test test_core_alloc test_core_handles)
    add_executable(${test} ${test}.c)
    target_link_libraries(${test} PRIVATE ugc-core-host)
    add_test(NAME ${test} COMMAND ${test})
endforeach()
