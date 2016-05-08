include_directories(../cereal/include/)

add_library(systemstate SHARED
  utils.cpp
  stateplugin.cpp
  protocol.cpp
  )

set(HEADERS
  protocol.h
  stateplugin.h
  utils.h
  )

set_target_properties(systemstate PROPERTIES SOVERSION 1 VERSION 1)

install(TARGETS systemstate LIBRARY DESTINATION /usr/lib)
install(FILES ${HEADERS} DESTINATION /usr/include/systemstate/)
