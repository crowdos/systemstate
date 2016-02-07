include_directories(../cereal/include/)

add_library(systemstate SHARED
  utils.cpp
  stateplugin.cpp
  protocol.cpp
  ${PROTO_SRCS}
  ${PROTO_HDRS}
  )

set_target_properties(systemstate PROPERTIES SOVERSION 0.0.1 VERSION 0.0.1)

install(TARGETS systemstate LIBRARY DESTINATION /usr/lib)
