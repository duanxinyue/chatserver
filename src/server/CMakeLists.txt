aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)
aux_source_directory(./message MESSAGE_LIST)
aux_source_directory(./util UTIL_LIST)
aux_source_directory(./net NET_LIST)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROJECT_SOURCE_DIR}/proto/chat.proto)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST} 
               ${MESSAGE_LIST} ${UTIL_LIST} ${NET_LIST} ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(ChatServer 
    muduo_net 
    muduo_base 
    mysqlclient 
    hiredis 
    pthread 
    crypto 
    ${PROTOBUF_LIBRARIES} 
    spdlog 
    ssl
)

install(TARGETS ChatServer DESTINATION bin)

target_compile_options(ChatServer PRIVATE 
    -O2 
    -DNDEBUG 
    -Wall 
    -Wextra 
    -Wpedantic 
    -Wno-unused-parameter
)

set_target_properties(ChatServer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
