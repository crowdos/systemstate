add_library(systemstate SHARED
  utils.cpp
  stateplugin.cpp
  protocol.cpp
  ${PROTO_SRCS}
  ${PROTO_HDRS}
  )

install(TARGETS systemstate DESTINATION /usr/lib)
