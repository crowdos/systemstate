find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS protocol.proto)

add_library(systemstate SHARED
  utils.cpp
  stateplugin.cpp
  protocol.cpp
  ${PROTO_SRCS}
  ${PROTO_HDRS}
  )

target_link_libraries(systemstate ${PROTOBUF_LIBRARIES})

install(TARGETS systemstate DESTINATION /usr/lib)
