cmake_minimum_required(VERSION 3.16)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/net)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/util)

add_executable(TestMessageCodec test_message_codec.cpp)
target_link_libraries(TestMessageCodec
    muduo_net
    muduo_base
    pthread
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
)

add_executable(TestChatServer test_chat_server.cpp)
target_link_libraries(TestChatServer
    muduo_net
    muduo_base
    pthread
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
)

add_test(NAME TestMessageCodec COMMAND TestMessageCodec)
add_test(NAME TestChatServer COMMAND TestChatServer)
