cmake_minimum_required(VERSION 3.16)
project(chatserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -Wpedantic -Wno-unused-parameter")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Wpedantic -Wno-unused-parameter -DNDEBUG")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/include/server/message)
include_directories(${PROJECT_SOURCE_DIR}/include/server/util)
include_directories(${PROJECT_SOURCE_DIR}/include/server/net)
include_directories(${PROJECT_SOURCE_DIR}/proto)

include_directories(/usr/local/include)
include_directories(/usr/include)
include_directories(/usr/include/hiredis)

link_directories(/usr/lib64/mysql)
link_directories(/usr/local/lib)
link_directories(/usr/lib/x86_64-linux-gnu)

add_subdirectory(src)

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
