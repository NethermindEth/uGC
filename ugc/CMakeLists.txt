# uGC build script
#
# Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
# Author: Maxim Menshikov <maksim.menshikov@nethermind.io>

cmake_minimum_required(VERSION 3.20)

# Set the project name
project(ugc-zero)

add_library(${PROJECT_NAME}
            OBJECT
            uGC.cpp
            uGCHandleManager.cpp
            uGCHandleStore.cpp
            uGCHeap.cpp)
target_compile_definitions(${PROJECT_NAME}
                           PUBLIC
                           -DTARGET_UNIX
                           -DHOST_64BIT)
target_include_directories(${PROJECT_NAME}
                           PUBLIC
                           .)
target_include_directories(${PROJECT_NAME}
                           PUBLIC
                           ${UGC_RUNTIME_INCLUDE_DIR}/src/native
                           ${UGC_RUNTIME_INCLUDE_DIR}/src/coreclr/inc
                           ${UGC_RUNTIME_INCLUDE_DIR}/src/coreclr/gc
                           ${UGC_RUNTIME_INCLUDE_DIR}/src/coreclr/gc/env)
install(TARGETS ${PROJECT_NAME}
        LIBRARY
            DESTINATION lib
        ARCHIVE
            DESTINATION lib
        OBJECTS
            DESTINATION lib
        RUNTIME
            DESTINATION bin)
